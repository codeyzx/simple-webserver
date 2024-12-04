counter = 1 -- Variabel global untuk menyimpan indeks

request = function()
    local data = string.format('{"data": "hello%d"}', counter)
    wrk.body = data
    wrk.headers["Content-Type"] = "application/json"
    counter = counter + 1
    if counter > 100 then
        counter = 1 -- Reset jika sudah mencapai 100
    end
    return wrk.format("POST")
end
