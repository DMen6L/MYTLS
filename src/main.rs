use clap::{Parser, Subcommand};

#[derive(Parser)]
#[command(name = "mytls")]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    Todo {
        #[command(subcommand)]
        command: TodoCommands,
    },

    Schedule {
        #[command(subcommand)]
        command: ScheduleCommands,
    },

    Today,
}

#[derive(Subcommand)]
enum TodoCommands {
    List,
    Add { title: String },
    Done { id: usize },
}

#[derive(Subcommand)]
enum ScheduleCommands {
    Today,
}

fn main() {
    let cli = Cli::parse();

    match cli.command {
        Commands::Todo { command } => match command {
            TodoCommands::List => println!("Listing tasks"),
            TodoCommands::Add { title } => {
                println!("Adding {title}");
            }
            TodoCommands::Done { id } => {
                println!("Done task with id {id}");
            }
        },

        Commands::Schedule { command } => match command {
            ScheduleCommands::Today => {
                println!("today's schedule");
            }
        },

        Commands::Today => {
            println!("today's schedule");
        }
    }
}
