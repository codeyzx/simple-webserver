-- Global variable to store request index
counter = 1

-- Function to create HTTP POST request
request = function()
    -- Create JSON data with incremented index
    local data = string.format('{"data": "hello%d"}', counter)
    -- Set request body with JSON data
    wrk.body = data
    -- Set Content-Type header to application/json
    wrk.headers["Content-Type"] = "application/json"
    -- Increment counter
    counter = counter + 1
    -- Reset counter if it reaches 100
    if counter > 100 then
        counter = 1
    end
    -- Return formatted POST request
    return wrk.format("POST")
end
