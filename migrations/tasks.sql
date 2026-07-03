CREATE TABLE tasks (
    id bigserial PRIMARY KEY,
    first_value integer NOT NULL,
    second_value integer DEFAULT 0,
    operation smallint NOT NULL,
    result integer NOT NULL,
    status smallint NOT NULL,
    UNIQUE (first_value, second_value, operation)
);
