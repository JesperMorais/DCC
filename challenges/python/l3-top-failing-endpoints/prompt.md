The on-call dashboard shows which endpoints are failing most. Each access-log line looks like `"GET /api/users 503 120ms"` (method, path, status code, duration), separated by single spaces. Write `top_failing_endpoints(lines, n)`.

- Only count **server errors**: status code `500` or higher.
- An endpoint is `"METHOD path"`, so `"GET /cart"` and `"POST /cart"` are different endpoints.
- Return up to `n` `(endpoint, count)` tuples, **most failures first**. Endpoints with the same count keep the order in which they **first failed**.
- `n` bigger than the number of failing endpoints returns them all, and `n == 0` or no failures gives `[]`.

```python
lines = [
    "GET /api/users 503 120ms",
    "POST /api/login 500 30ms",
    "GET /api/users 200 15ms",
    "GET /api/users 502 98ms",
]
top_failing_endpoints(lines, 1)  # [("GET /api/users", 2)]
top_failing_endpoints(lines, 5)  # [("GET /api/users", 2), ("POST /api/login", 1)]
```
