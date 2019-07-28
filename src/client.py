from recept_pb2 import CocktailSetup, Recept, Ingredient, Liquid
import requests


if __name__ == "__main__":

    print("Hello, I am your Cocktail Maker.")

    machine = "http://192.168.178.111"

    liquids = ['Orangensaft', 'Maracuja', 'Grenadine', 'Limettensaft']

    answer = requests.get(machine + "/setup", headers={"content-type":"application/x-protobuf"})

    print(answer.headers)
    print(answer.content)

    # Decode the byte message
    setup = CocktailSetup()
    #setup.ParseFromString(answer.content)

    print(len(setup.liquids))

    setup = CocktailSetup()

    for i in range(4):
        liq = setup.liquids.add()
        liq.name = liquids[i]
        liq.pump_id = i

    raw_bytes = setup.SerializeToString()

    print(raw_bytes + b'\x00')

    answer = requests.post(machine + "/setup", data= {
        'proto' : raw_bytes + b'\x00'
    })

    print(answer.content.decode('utf-8'))