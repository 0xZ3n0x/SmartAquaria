import os
import subprocess
import time


class VirtualUart:
    @staticmethod
    def start(port_a: str, port_b: str) -> subprocess.Popen:
        """Start a socat virtual UART pair. Returns the socat process."""
        VirtualUart.cleanup(port_a, port_b)
        return _start(port_a, port_b)

    @staticmethod
    def stop(proc: subprocess.Popen) -> None:
        """Terminate a socat process."""
        try:
            proc.terminate()
            proc.wait(timeout=3)
        except (OSError, subprocess.TimeoutExpired):
            pass

    @staticmethod
    def cleanup(*paths: str) -> None:
        """Remove stale virtual port files left from a previous run."""
        for path in paths:
            if os.path.exists(path):
                try:
                    os.unlink(path)
                except OSError:
                    pass
        time.sleep(0.5)


# ---------------------------------------------------------------------------
# Module-level helpers (private)
# ---------------------------------------------------------------------------

def _start(port_a: str, port_b: str) -> subprocess.Popen:
    proc = subprocess.Popen(
        ["socat", "-d", "-d",
         f"pty,raw,echo=0,link={port_a}",
         f"pty,raw,echo=0,link={port_b}"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    time.sleep(1)

    if not _wait_for(port_a) or not _wait_for(port_b):
        proc.terminate()
        raise RuntimeError(f"Virtual UART ports did not appear: {port_a} <-> {port_b}")

    print(f"Virtual UART: {port_a} <-> {port_b}  (PID {proc.pid})")
    return proc


def _wait_for(path: str, timeout: float = 3.0) -> bool:
    start_time = time.time()
    while time.time() - start_time < timeout:
        if os.path.exists(path):
            return True
        time.sleep(0.1)
    return False
