import requests

BASE = "http://192.168.0.12:5001/"

data = [{"name": "Esp_Luz", "ip": "1231331231", "ip2": 1233211, "connected": 1, "function":"Luz", "value": 1},
        {"name": "Esp_Portao", "ip": "1231311312321", "ip2": 123213132, "connected": 0, "function":"Portao","value": 0},
        {"name": "Esp_Ar", "ip": "12311112331", "ip2": 13123, "connected": 1, "function":"Ar","value": 21}]

for i in range(len(data)):
    response = requests.put(BASE + "client/" + str(i), data[i])
    print(response.json())


response = requests.get(BASE + "client/2")
print(response.json())

#response = requests.patch(BASE + "client/2", {"value": 19})
#print(response.json())

response = requests.get(BASE + "clients")
print(response.json())

