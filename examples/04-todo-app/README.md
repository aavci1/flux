# Todo App Example

Demonstrates:
- Dynamic lists with reactive State
- Lambda functions for event handlers
- Adding and removing items from lists
- Nested components with declarative syntax

## Features

- **Add Random Todo**: Click to add a random task to the list
- **Delete Todo**: Each todo item has a delete button
- **Reactive Counter**: Header shows current task count
- **Clean UI**: Styled with shadows, colors, and spacing

## Key Concepts

### Dynamic Lists
```cpp
State<std::vector<TodoItem>> todos = {...};
```

### Lambda Closures
```cpp
.onClick = [&, todoId = todo.id]() {
    // Capture by reference and by value
    std::vector<TodoItem> updated = todos;
    // Modify and update state
    todos = updated;
}
```

### Reactive UI
When `todos` state changes, the UI automatically rebuilds and re-renders the list.

