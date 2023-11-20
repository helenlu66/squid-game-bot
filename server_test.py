import numpy as np
import time
import requests
import random
from PIL import Image
from io import BytesIO

esp32_url = "http://10.247.137.84"
state = requests.get(esp32_url+"/start")
# state = requests.get("http://10.247.137.84/start")
print(state)
tgt = (1,1)
# data = '{"servo":"shootmode",'+'"X":1}'
data = {"servo":"shoot","x":0, "y":0}
headers = {'Content-type': 'application/json', 'Accept': 'text/plain'}
# requests.post(esp32_url+"/pose",data)
requests.post(esp32_url+"/post-message",json=data)
# requests.post(esp32_url+"/pose?servo=shootmode")
