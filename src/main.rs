use chrono::DateTime;
use chrono_tz::GMT;
use feed_rs::parser;
use pyo3::prelude::*;
use rusqlite::{Connection, Result};
use std::collections::HashMap;
use std::path::Path;
use std::process::Command;
use std::process::Stdio;
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

fn post_process(path: &Path) {
    let dir = dirs::video_dir().expect("video dir not found");
    let out_path = dir.join("podcast").join(path.file_name().as_ref().unwrap());
    let path_str = path.to_str().unwrap();
    if path_str.contains("Wisteria") {
        Command::new("ffmpeg")
            .arg("-ss")
            .arg("40")
            .arg("-i")
            .arg(path)
            .arg(&out_path)
            .stdout(Stdio::null())
            .stderr(Stdio::null())
            .output()
            .expect("failed to execute process");
        std::fs::remove_file(path).expect("error deleting");
    } else if path_str.contains("Diario de Ucrania") {
        Command::new("ffmpeg")
            .arg("-ss")
            .arg("4")
            .arg("-i")
            .arg(path)
            .arg(&out_path)
            .stdout(Stdio::null())
            .stderr(Stdio::null())
            .output()
            .expect("failed to execute process");
        std::fs::remove_file(path).expect("error deleting");
    } else {
        std::fs::copy(path, out_path).expect("error copying");
        std::fs::remove_file(path).expect("error deleting");
    }
}

async fn get_request(url: &str, last: i64) -> Result<String, ()> {
    let client = reqwest::Client::new();
    let dt = DateTime::from_timestamp(last, 0).expect("invalid timestamp");
    let Ok(resp) = client
        .get(url)
        .header(
            reqwest::header::IF_MODIFIED_SINCE,
            format!(
                "{}",
                dt.with_timezone(&GMT).format("%a, %d %b %Y %H:%M:%S %Z")
            ),
        )
        .send()
        .await
    else {
        println!("error requesting {}", url);
        return Err(());
    };
    let code = resp.status().as_u16();
    if code == 304 {
        //No changed
        return Err(());
    };
    if code != 200 {
        println!("error requesting {}: {}", url, code);
        return Err(());
    };
    Ok(resp.text().await.unwrap())
}

async fn process(url: &str, name: &str, last: i64) -> Result<feed_rs::model::Feed, ()> {
    let Ok(resp) = get_request(url, last).await else {
        return Err(());
    };
    let Ok(feed) = parser::parse(resp.as_bytes()) else {
        println!("error processing {}", name);
        return Err(());
    };
    Ok(feed)
}

async fn blog(conn: &Connection, last: i64) -> Result<i64, Box<dyn std::error::Error>> {
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
        let Ok(feed) = process(data.url.as_str(), data.name.as_str(), last).await else {
            continue;
        };
        for entry in feed.entries {
            let published = entry.published.unwrap().timestamp();
            if published > last {
                if published > new_date {
                    new_date = published;
                }
            } else {
                break;
            }
            let link = &entry.links.last().unwrap().href;
            println!("{}", link);
        }
        thread::sleep(Duration::from_millis(500));
    }
    Ok(new_date)
}
async fn podcast(
    conn: &Connection,
    last: i64,
    dry_run: bool,
) -> Result<i64, Box<dyn std::error::Error>> {
    let mut new_date = last;
    let mut stmt = conn.prepare("SELECT name, url FROM podcast")?;
    let data_iter = stmt.query_map([], |row| {
        Ok(Data {
            name: row.get(0)?,
            url: row.get(1)?,
        })
    })?;

    for it in data_iter {
        let data = it?;
        let Ok(feed) = process(data.url.as_str(), data.name.as_str(), last).await else {
            continue;
        };
        for entry in feed.entries {
            let published = entry.published.unwrap().timestamp();
            if published > last {
                if published > new_date {
                    new_date = published;
                }
            } else {
                break;
            }
            let title = entry.title.unwrap().content;
            let media = entry.media.last().unwrap();
            let link = media.content.last().unwrap().url.as_ref().unwrap();
            if dry_run {
                println!("{}", link);
            } else {
                let resp = reqwest::get(link.as_str()).await.expect("request failed");
                let body = resp.bytes().await.expect("body invalid");
                let filename = str::replace(&title, "/", "-") + ".opus";
                let dir = Path::new("/tmp/");
                let path = dir.join(filename);
                std::fs::write(path.clone(), &body).expect("write failed");
                post_process(&path);
            }
        }
        thread::sleep(Duration::from_millis(500));
    }
    Ok(new_date)
}
async fn youtube(
    conn: &Connection,
    last: i64,
    dry_run: bool,
) -> Result<i64, Box<dyn std::error::Error>> {
    let prefix = "https://www.youtube.com/feeds/videos.xml?channel_id=";
    let mut new_date = last;
    let mut map: HashMap<String, Vec<String>> = HashMap::new();
    let mut stmt = conn.prepare(
        "select id,name,folder from youtube where download = true and regex_id is null;",
    )?;
    let data_iter = stmt.query_map([], |row| {
        Ok(Channel {
            id: row.get(0)?,
            name: row.get(1)?,
            folder: row.get(2)?,
        })
    })?;
    for it in data_iter {
        let channel = it?;
        let url = prefix.to_owned() + channel.id.as_str();
        let Ok(feed) = process(url.as_str(), channel.name.as_str(), last).await else {
            continue;
        };
        for entry in feed.entries {
            let published = entry.published.unwrap().timestamp();
            if published > last {
                if published > new_date {
                    new_date = published;
                }
            } else {
                break;
            }
            let link = &entry.links.last().unwrap().href;
            map.entry(channel.folder.to_owned())
                .and_modify(|e| e.push(link.to_owned()))
                .or_insert_with(|| vec![link.to_owned()]);
        }
        thread::sleep(Duration::from_millis(600));
    }
    let mut stmt = conn.prepare("select youtube.id,youtube.name,folder,value,negative_match from youtube inner join regex on regex_id=regex.id where download = true;")?;
    let data_iter = stmt.query_map([], |row| {
        Ok((
            Channel {
                id: row.get(0)?,
                name: row.get(1)?,
                folder: row.get(2)?,
            },
            Regex {
                regex: row.get(3)?,
                negative_match: row.get(4)?,
            },
        ))
    })?;
    for it in data_iter {
        let (channel, regex) = it?;
        let url = prefix.to_owned() + channel.id.as_str();
        let Ok(feed) = process(url.as_str(), channel.name.as_str(), last).await else {
            continue;
        };
        for entry in feed.entries {
            let published = entry.published.unwrap().timestamp();
            if published > last {
                if published > new_date {
                    new_date = published;
                }
            } else {
                break;
            }
            let title = entry.title.unwrap().content;
            let mut matched = false;
            if (regex.negative_match && !title.contains(&regex.regex))
                || (!regex.negative_match && title.contains(&regex.regex))
            {
                matched = true;
            }

            if matched {
                let link = &entry.links.last().unwrap().href;
                map.entry(channel.folder.to_owned())
                    .and_modify(|e| e.push(link.to_owned()))
                    .or_insert_with(|| vec![link.to_owned()]);
            }
        }
        thread::sleep(Duration::from_millis(600));
    }
    if !map.is_empty() {
        Python::with_gil(|py| {
            let py_app = include_str!(concat!(env!("CARGO_MANIFEST_DIR"), "/youtube.py"));
            let download: Py<PyAny> = PyModule::from_code_bound(py, py_app, "", "")
                .unwrap()
                .getattr("download")
                .unwrap()
                .into();
            for (folder, links) in map {
                if dry_run {
                    println!("downloading {:?} to {}", links, folder);
                } else {
                    let _ = download.call1(py, (true, folder, links)).unwrap();
                }
            }
        });
    }
    Ok(new_date)
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let dir = dirs::video_dir().expect("video dir not found");
    let conn = Connection::open(dir.join("data.db"))?;
    let last: i64 = conn.query_row("select unix_time_stamp from last;", [], |row| row.get(0))?;
    let dry_run = false;
    let mut new_last = last;
    let last_blog = blog(&conn, last).await.expect("failed processing blog");
    if last_blog > last {
        new_last = last_blog;
    }
    let last_podcast = podcast(&conn, last, dry_run)
        .await
        .expect("failed processing podcast");
    if last_podcast > new_last {
        new_last = last_podcast;
    }
    let last_youtube = youtube(&conn, last, dry_run)
        .await
        .expect("failed processing youtube");
    if last_youtube > new_last {
        new_last = last_youtube;
    }
    if new_last != last {
        if dry_run {
            println!("updating timestamp to {}", new_last);
        } else {
            conn.execute("update last set unix_time_stamp = ?;", [new_last])
                .expect("failed to update last timestamp");
        }
    }
    Ok(())
}
