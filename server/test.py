import requests

BASE = "http://192.168.0.2:5001/"

data = [{"name": "Esp_Luz", "ip": "1231331231", "ip2": 1233211, "connected": 1, "function":"Luz", "value": 0, "action": 1, "act_value": 0},
        {"name": "Esp_Porta", "ip": "1231311312321", "ip2": 123213132, "connected": 1, "function":"Porta","value": 0, "action": 0, "act_value": 0},
        {"name": "Esp_Ar", "ip": "12311112331", "ip2": 13123, "connected": 1, "function":"Ar","value": 16, "action": 1, "act_value": 16}]

for i in range(len(data)):
    response = requests.put(BASE + "client/" + str(i), data[i])
    print(response.json())


#response = requests.get(BASE + "client/2")
#print(response.json())

#response = requests.patch(BASE + "client/2", {"name": "Esp_Ar", "ip": "12311112331", "ip2": 13123, "connected": 0, "function":"Ar", "value": 25, "action": 0, "act_value": 21})
#print(response.json())

#response = requests.get(BASE + "clients")
#print(response.json())

