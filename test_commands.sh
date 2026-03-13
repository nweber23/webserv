#!/bin/bash

# GET request
curl -s -o /dev/null -w "GET /: %{http_code}\n" http://localhost:8080/

# GET non-existent
curl -s -o /dev/null -w "GET /fake: %{http_code}\n" http://localhost:8080/fake

# POST request
curl -s -o /dev/null -w "POST /: %{http_code}\n" -X POST http://localhost:8080/

# DELETE request
curl -s -o /dev/null -w "DELETE /: %{http_code}\n" -X DELETE http://localhost:8080/

# Unknown method
curl -s -o /dev/null -w "UNKNOWN /: %{http_code}\n" -X UNKNOWN http://localhost:8080/
