import requests

BASE = "http://192.168.0.10:5001/"

data = [{"name": "legal", "views": 1000, "likes":100},
        {"name": "mais legal", "views": 2000, "likes":100},
        {"name": "mais legal ainda", "views": 3000, "likes":200}]

for i in range(len(data)):
    response = requests.put(BASE + "video/" + str(i), data[i])
    print(response.json())


response = requests.get(BASE + "video/2")
print(response.json())

response = requests.patch(BASE + "video/2", {"views": 4000})
print(response.json())

response = requests.get(BASE + "videos")
print(response.json())

