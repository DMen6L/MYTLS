use std::env;
use std::fs::{self, OpenOptions};
use std::io::{self, Write};

fn append_task(task: &str) -> io::Result<()> {
    let mut file = OpenOptions::new()
        .create(true)
        .append(true)
        .open("tasks.txt")?;

    writeln!(file, "{task}")?;
    Ok(())
}

fn main() -> io::Result<()> {
    // Skip first since it is the executable item
    let arguments: Vec<String> = env::args().skip(1).collect();
    let mut tasks: Vec<String> = Vec::new();

    match fs::read_to_string("tasks.txt") {
        Ok(content) => {
            for task in content.lines() {
                tasks.push(task.to_string());
            }
        },
        Err(error) if error.kind() == io::ErrorKind::NotFound => {}
        Err(error) => return Err(error),
    };

    let Some((command, rest)) = arguments.split_first() else {
        return Err(io::Error::new(
                io::ErrorKind::InvalidInput,
                "missing command",
            ));
    };

    match command.as_str() {
        "list" if rest.is_empty() => {
            if tasks.is_empty() {
                println!("No tasks yet.");
                return Ok(());
            }

            for(index, task) in tasks.iter().enumerate() {
                println!("{}: {}", index + 1, task);
            }
        },
        "add" if rest.len() == 1 => append_task(&rest[0])?,
        "done" if rest.len() == 1 => println!("Completing task # {}", rest[0]),
        "remove" if rest.len() == 1 => println!("Removing task # {}", rest[0]),
        _ => println!("Invalid command or arguments."),
    }

    Ok(())
}
