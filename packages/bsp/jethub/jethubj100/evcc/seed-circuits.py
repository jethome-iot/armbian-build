"""Add the J100 station circuit without replacing existing evcc settings."""

from pathlib import Path
import sqlite3
import sys


def main():
    path = Path(sys.argv[1])
    circuits = Path(sys.argv[2]).read_text()
    path.parent.mkdir(parents=True, exist_ok=True)
    with sqlite3.connect(path) as database:
        database.execute("CREATE TABLE IF NOT EXISTS settings (key TEXT PRIMARY KEY, value TEXT)")
        database.execute(
            "INSERT OR IGNORE INTO settings (key, value) VALUES ('circuits', ?)",
            (circuits,),
        )
    database.close()
    path.chmod(0o600)


if __name__ == "__main__":
    main()
