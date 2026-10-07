mod helpers;

use std::fs::OpenOptions;
use std::io::{self, BufRead, BufReader};
use std::{env, fs};

use crate::helpers::task::{self, Task};

fn save_tasks(tasks: &Vec<Task>, file_name: String) -> io::Result<()> {
    let mut contents = String::new();

    for task in tasks {
        contents.push_str(&&task.serialize());
        contents.push('\n');
    }

    fs::write(file_name, contents)?;

    Ok(())
}

fn main() -> io::Result<()> {
    // args[0] otherwise would be command name
    let args: Vec<String> = env::args().skip(1).collect();

    if args.is_empty() {
        println!("Usage: mytls <add|list|done|remove> ...");
        return Ok(());
    }

    let mut tasks: Vec<Task> = Vec::new();
    let file = OpenOptions::new()
        .create(true)
        .read(true)
        .append(true)
        .open("tasks.txt")?;

    let reader = BufReader::new(file);

    for line in reader.lines() {
        let line: String = line?;

        // Parse read task
        let task = match task::read_task(&line) {
            Ok(task) => task,
            Err(error) => {
                println!("Failed to read task from file: {error}");
                return Ok(());
            }
        };

        tasks.push(task);
    }

    match args[0].as_str() {
        "list" => {
            if tasks.is_empty() {
                println!("No tasks.");
                return Ok(());
            }

            for task in tasks.iter() {
                println!("{}", task.display());
            }
        }

        "add" => {
            if args.len() != 2 {
                println!("add can only have 2 arguments.");
                return Ok(());
            }

            let new_task = Task::new(tasks.len(), args[1].to_string());
            tasks.push(new_task);
        }

        "done" => {
            if args.len() != 2 {
                println!("done can have only 2 arguments.");
                return Ok(());
            }

            // Parsing id of the done task
            let id: usize = match args[1].parse::<usize>() {
                Ok(id) => id,
                Err(_) => {
                    println!("done needs to be a positive number");
                    return Ok(());
                }
            };

            tasks[id].complete();
        }

        "remove" => {
            if args.len() != 2 {
                println!("remove can only have 2 arguments");
                return Ok(());
            }

            // Parsing id of the done task
            let id: usize = match args[1].parse::<usize>() {
                Ok(id) => id,
                Err(_) => {
                    println!("done needs to be a positive number");
                    return Ok(());
                }
            };

            tasks.remove(id);
        }
        _ => println!("No such command."),
    }

    save_tasks(&tasks, "tasks.txt".to_string())
}
