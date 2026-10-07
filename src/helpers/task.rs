pub struct Task {
    id: usize,
    title: String,
    completed: bool,
}

pub fn read_task(line: &str) -> Result<Task, String> {
    let parts: Vec<&str> = line.split("|").collect();

    if parts.len() != 3 {
        return Err("Invalid task format".to_string());
    }

    let id: usize = parts[0]
        .parse()
        .map_err(|_| "Invalid task id.".to_string())?;

    let completed: bool = parts[1]
        .parse()
        .map_err(|_| "Invalid task completed value.".to_string())?;

    let title: String = parts[2].parse().map_err(|_| "Invalid task title.")?;

    Ok(Task {
        id,
        title,
        completed,
    })
}

impl Task {
    pub fn new(id: usize, title: String) -> Self {
        Self {
            id,
            title,
            completed: false,
        }
    }

    pub fn complete(&mut self) {
        self.completed = true;
    }

    pub fn display(&self) -> String {
        let status = if self.completed { "x" } else { " " };

        format!("{} [{}] {}", self.id, status, self.title)
    }

    pub fn serialize(&self) -> String {
        format!("{}|{}|{}", self.id, self.completed, self.title)
    }
}
