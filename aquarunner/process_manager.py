import os
import signal
import subprocess
import sys
import time


class ProcessManager:
    @staticmethod
    def start(cmd: list[str], name: str, cwd: str | None = None) -> subprocess.Popen:
        print(f"Starting {name}: {' '.join(cmd)}")
        proc = subprocess.Popen(
            cmd,
            stdout=None,   # inherit — output visible in the launching terminal
            stderr=None,
            cwd=cwd,
            env={**os.environ, "PYTHONUNBUFFERED": "1"},
            start_new_session=True,
        )
        print(f"Started {name}  (PID {proc.pid})")
        return proc

    @staticmethod
    def stop(proc: subprocess.Popen, name: str) -> None:
        print(f"Stopping {name}...")
        try:
            os.kill(proc.pid, signal.SIGTERM)
            proc.wait(timeout=3)
            return
        except (OSError, subprocess.TimeoutExpired):
            pass
        try:
            os.kill(proc.pid, signal.SIGKILL)
        except OSError:
            pass

    @staticmethod
    def monitor(procs: dict[str, subprocess.Popen]) -> tuple[str, int]:
        """Block until any process exits. Returns (name, exit_code) of the dead process."""
        while True:
            time.sleep(1)
            for name, proc in procs.items():
                rc = proc.poll()
                if rc is not None:
                    return name, rc

    @staticmethod
    def run_until_exit(procs: dict[str, subprocess.Popen]) -> None:
        """Install signal handlers and block until any process dies or a signal arrives."""
        signal.signal(signal.SIGINT,  lambda *_: sys.exit(0))
        signal.signal(signal.SIGTERM, lambda *_: sys.exit(0))

        print("Press Ctrl+C to stop...")
        dead, returncode = ProcessManager.monitor(procs)
        print(f"\n{dead} process died unexpectedly (exit code: {returncode}).")
        sys.exit(1)
