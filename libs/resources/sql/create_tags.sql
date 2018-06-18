CREATE TABLE IF NOT EXISTS tags (
    id INTEGER PRIMARY KEY
,   name TEXT
,   storage_id INTEGER
,   active INTEGER
,   FOREIGN KEY(storage_id) REFERENCES storages(id)
);