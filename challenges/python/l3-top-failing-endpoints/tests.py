LINES = [
    "GET /api/users 503 120ms",
    "POST /api/login 500 30ms",
    "GET /api/users 200 15ms",
    "GET /api/users 502 98ms",
    "GET /health 200 1ms",
]

def test_most_failures_first():
    assert top_failing_endpoints(LINES, 1) == [("GET /api/users", 2)]

def test_n_larger_than_the_number_of_endpoints():
    assert top_failing_endpoints(LINES, 10) == [("GET /api/users", 2), ("POST /api/login", 1)]

def test_only_status_500_and_above_counts():
    lines = ["GET /a 499 1ms", "GET /b 500 1ms", "GET /c 404 1ms", "GET /d 599 1ms"]
    assert top_failing_endpoints(lines, 10) == [("GET /b", 1), ("GET /d", 1)]

def test_method_is_part_of_the_endpoint():
    lines = ["GET /cart 500 1ms", "POST /cart 500 1ms", "POST /cart 503 1ms"]
    assert top_failing_endpoints(lines, 2) == [("POST /cart", 2), ("GET /cart", 1)]

def test_ties_keep_first_failure_order():
    lines = ["GET /z 500 1ms", "GET /a 500 1ms", "GET /m 500 1ms"]
    assert top_failing_endpoints(lines, 3) == [("GET /z", 1), ("GET /a", 1), ("GET /m", 1)]

def test_nothing_to_report():
    assert top_failing_endpoints(LINES, 0) == []
    assert top_failing_endpoints(["GET / 200 1ms"], 3) == []
    assert top_failing_endpoints([], 3) == []
