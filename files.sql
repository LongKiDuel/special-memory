DROP TABLE IF EXISTS file_changes;
DROP TABLE IF EXISTS machines;

CREATE TABLE machines (
    id SERIAL PRIMARY KEY,
    name TEXT NOT NULL
);

CREATE TABLE file_changes (
    -- id SERIAL PRIMARY KEY,
    id INT GENERATED ALWAYS AS IDENTITY,
    machine_id INTEGER,
    filename TEXT NOT NULL,
    filepath TEXT NOT NULL,
    filesize INTEGER NOT NULL,
    last_modify_time TIMESTAMP WITH TIME ZONE  NOT NULL,
    creation_time TIMESTAMP WITH TIME ZONE,
    file_hash TEXT,
    change_type TEXT,
    user_info TEXT,
    file_type TEXT,
    last_check_time TIMESTAMP WITH TIME ZONE DEFAULT LOCALTIMESTAMP,
    last_check_local_time TIME DEFAULT LOCALTIMESTAMP,

    PRIMARY KEY(id),
    CONSTRAINT fk_machine_id
        FOREIGN KEY(machine_id)
            REFERENCES machines(id)
);


-- Example of creating an index
CREATE INDEX idx_filename ON file_changes(filename);

