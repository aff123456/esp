import requests

BASE = "http://192.168.0.10:5001/"

data = [{"name": "Esp_Luz", "connected": 1, "function":"Luz", "value": 1},
        {"name": "Esp_Portao", "connected": 1, "function":"Portao","value": 0},
        {"name": "Esp_Ar", "connected": 0, "function":"Ar","value": 21}]

for i in range(len(data)):
    response = requests.put(BASE + "client/" + str(i), data[i])
    print(response.json())


response = requests.get(BASE + "client/2")
print(response.json())

#response = requests.patch(BASE + "client/2", {"value": 19})
#print(response.json())

response = requests.get(BASE + "clients")
print(response.json())

