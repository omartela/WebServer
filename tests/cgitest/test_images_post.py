#!/usr/bin/env pytest
from pathlib import Path
import requests

def test_images_post():    
	files = {'file': ('filename.txt', b"dummy data\n")}
	response = requests.post("http://127.0.0.2:8004/images/", files=files)
	assert response.status_code in (200, 201)