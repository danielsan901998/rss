use rusqlite::{Connection, Result};
use feed_rs::parser;
use pyo3::prelude::*;
use chrono::{DateTime,Utc};
use std::collections::HashMap;
use std::{thread, time::Duration};

#[derive(Debug)]
struct Data {
    name: String,
    url: String,
}
#[derive(Debug)]
struct Channel {
    id: String,
    name: String,
    folder: String,
}
#[derive(Debug)]
struct Regex {
    regex: String,
    negative_match: bool,
}

async fn blog(conn : &Connection, last : DateTime<Utc>) -> Result<i64, Box<dyn std::error::Error>> {
    let mut new_date = last;
    let mut stmt = conn.prepare("SELECT name, url FROM blog")?;
    let data_iter = stmt.query_map([], |row| {
        Ok(Data {
            name: row.get(0)?,
            url: row.get(1)?,
        })
    })?;

    for it in data_iter {
        let data = it?;
        let Ok(resp) = reqwest::get(data.url).await else{
            println!("error requesting {}", data.name);
            continue;
        };
        let Ok(feed) = parser::parse(resp.text().await?.as_bytes()) else{
            println!("error processing {}", data.name);
            continue;
        };
        for entry in feed.entries {
            let published = entry.published.unwrap();
            if published > last {
                if published > new_date {
                    new_date = published;
                }
            }else{
                break;
            }
            let link = &entry.links.last().unwrap().href;
            println!("link {}", link);
        }
        thread::sleep(Duration::from_millis(500));
    }
    Ok(new_date.timestamp())
}
async fn podcast(conn : &Connection, last : DateTime<Utc>, dry_run : bool) -> Result<i64, Box<dyn std::error::Error>> {
    let mut new_date = last;
    let dir = dirs::video_dir().expect("video dir not found");
    let mut stmt = conn.prepare("SELECT name, url FROM podcast")?;
    let data_iter = stmt.query_map([], |row| {
        Ok(Data {
            name: row.get(0)?,
            url: row.get(1)?,
        })
    })?;

    for it in data_iter {
        let data = it?;
        let Ok(resp) = reqwest::get(data.url).await else{
            println!("error requesting {}", data.name);
            continue;
        };
        let Ok(feed) = parser::parse(resp.text().await?.as_bytes()) else{
            println!("error processing {}", data.name);
            continue;
        };
        for entry in feed.entries {
            let published = entry.published.unwrap();
            if published > last {
                if published > new_date {
                    new_date = published;
                }
            }else{
                break;
            }
            let title = entry.title.unwrap().content;
            let media = entry.media.last().unwrap();
            let link = media.content.last().unwrap().url.as_ref().unwrap();
            if dry_run {
                println!("link {}", link);
            }else{
                let resp = reqwest::get(link.as_str()).await.expect("request failed");
                let body = resp.bytes().await.expect("body invalid");
                let filename = str::replace(&title, "/", "-")+".mp3";
                std::fs::write(dir.join("podcast").join(filename), &body).expect("write failed");
            }
        }
        thread::sleep(Duration::from_millis(500));
    }
    Ok(new_date.timestamp())
}
async fn youtube(conn : &Connection, last : DateTime<Utc>, dry_run : bool) -> Result<i64, Box<dyn std::error::Error>> {
    let prefix = "https://www.youtube.com/feeds/videos.xml?channel_id=";
    let mut new_date = last;
    let mut map : HashMap<String,Vec<String>> = HashMap::new();
    let mut stmt = conn.prepare("select id,name,folder from youtube where download = true and regex_id is null;")?;
    let data_iter = stmt.query_map([], |row| {
        Ok(Channel {
            id: row.get(0)?,
            name: row.get(1)?,
            folder: row.get(2)?,
        })
    })?;
    for it in data_iter {
        let channel = it?;
        let Ok(resp) = reqwest::get(prefix.to_owned()+channel.id.as_str()).await else{
            println!("error requesting {}", channel.name);
            continue;
        };
        let Ok(feed) = parser::parse(resp.text().await?.as_bytes()) else{
            println!("error processing {}", channel.name);
            continue;
        };
        for entry in feed.entries {
            let published = entry.published.unwrap();
            if published > last {
                if published > new_date {
                    new_date = published;
                }
            }else{
                break;
            }
            let link = &entry.links.last().unwrap().href;
            if dry_run {
                println!("link {}", link);
            }else{
                map.entry(channel.folder.to_owned()).and_modify(|e|e.push(link.to_owned())).or_insert_with(|| vec![link.to_owned()]);
            }
        }
        thread::sleep(Duration::from_millis(600));
    }
    let mut stmt = conn.prepare("select youtube.id,youtube.name,folder,value,negative_match from youtube inner join regex on regex_id=regex.id where download = true;")?;
    let data_iter = stmt.query_map([], |row| {
        Ok((Channel {
            id: row.get(0)?,
            name: row.get(1)?,
            folder: row.get(2)?,
        },
        Regex {
            regex: row.get(3)?,
            negative_match: row.get(4)?,
        }))
    })?;
    for it in data_iter {
        let (channel,regex) = it?;
        let Ok(resp) = reqwest::get(prefix.to_owned()+channel.id.as_str()).await else{
            println!("error requesting {}", channel.name);
            continue;
        };
        let Ok(feed) = parser::parse(resp.text().await?.as_bytes()) else{
            println!("error processing {}", channel.name);
            continue;
        };
        for entry in feed.entries {
            let published = entry.published.unwrap();
            if published > last {
                if published > new_date {
                    new_date = published;
                }
            }else{
                break;
            }
            let title = entry.title.unwrap().content;
            let mut matched = false;
            if regex.negative_match && !title.contains(&regex.regex) {
                matched = true;
            }
            else if !regex.negative_match && title.contains(&regex.regex) {
                matched = true;
            }

            if matched {
                let link = &entry.links.last().unwrap().href;
                if dry_run {
                    println!("link {}", link);
                }else{
                    map.entry(channel.folder.to_owned()).and_modify(|e|e.push(link.to_owned())).or_insert_with(|| vec![link.to_owned()]);
                }
            }
        }
        thread::sleep(Duration::from_millis(600));
    }
    if !map.is_empty() {
        Python::with_gil(|py| {
            let py_app = include_str!(concat!(
                    env!("CARGO_MANIFEST_DIR"),
                    "/youtube.py"
                    ));
            let download: Py<PyAny> = PyModule::from_code_bound(py, py_app, "", "").unwrap() .getattr("download").unwrap().into();
            for (folder, links) in map {
                if dry_run {
                    println!("downloading {:?} to {}", links, folder);
                }else{
                    let _ = download.call1(py,(true,folder,links)).unwrap();
                }
            }
        });
    }
    Ok(new_date.timestamp())
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let dir = dirs::video_dir().expect("video dir not found");
    let conn = Connection::open(dir.join("data.db"))?;
    let last : i64 = conn.query_row("select unix_time_stamp from last;",[], |row| row.get(0))?;
    let dt = DateTime::from_timestamp(last, 0).expect("invalid timestamp");
    let dry_run = false;
    let mut new_last = last;
    let last_blog = blog(&conn,dt).await.expect("failed processing blog");
    if last_blog > last {
        new_last = last_blog;
    }
    let last_podcast = podcast(&conn,dt,dry_run).await.expect("failed processing podcast");
    if last_podcast > last {
        new_last = last_podcast;
    }
    let last_youtube = youtube(&conn,dt,dry_run).await.expect("failed processing youtube");
    if last_youtube > last {
        new_last = last_youtube;
    }
    if new_last != last {
        if dry_run {
            println!("updating timestamp to {}", new_last);
        }else{
            conn.execute("update last set unix_time_stamp = ?;",[new_last]).expect("failed to update last timestamp");
        }
    }
    Ok(())
}
