import json
import os
import re
import requests
from cobs import cobs
from recept_pb2 import CocktailSetup


def list_available_recipes(path):

    items = os.listdir(path)
    items = map( lambda i : os.path.join(path, i), items)
    items = filter( lambda i : os.path.isfile(i), items)
    items = filter(lambda i: re.match(".*\.json", i), items)

    return list(items)


def load_recipe(path):

    with open(path, 'r') as file:
        recipe = json.load(file)

    return path, recipe

def make_recipe(recipe):
    print("Send make request")
    machine = "http://192.168.178.111"
    raw_bytes = recipe.SerializeToString()
    raw_bytes = cobs.encode(raw_bytes)
    print(raw_bytes)

    answer = requests.post(machine + "/make", data= {'proto' : raw_bytes + b'\x00'})
    print(answer)
    print(answer.content)

    return answer


def load_current_cocktail_setup():

    print("Loading Setup")

    machine = "http://192.168.178.111"
    answer = requests.get(machine + "/setup",headers= {'content-type' : 'application/x-protobuf'})
    print(answer)

    setup = CocktailSetup()
    setup.ParseFromString(answer.content)
    return setup

def save_setup(setup):
    machine = "http://192.168.178.111"
    print("Saving Setup...")
    raw_bytes = setup.SerializeToString()

    print(raw_bytes)

    answer = requests.post(machine + "/setup", data= {
        'proto' : raw_bytes + b'\x00'
    })
    print(answer)

def get_progress():
    machine = "http://192.168.178.111"
    answer = requests.get(machine + "/progress")
    return answer.content, answer.status_code

def clean():
    machine = "http://192.168.178.111"
    requests.get(machine + "/clean")

def adjust():
    machine = "http://192.168.178.111"
    requests.get(machine + "/startup")

