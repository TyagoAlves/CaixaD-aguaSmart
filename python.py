import json
import pandas as pd

# Carregar o arquivo JSON (se estiver em um arquivo separado, use: with open("arquivo.json") as f: data = json.load(f))
json_string = """COLE_SEU_JSON_AQUI"""
data = json.loads(json_string)

# Criar uma lista com os dados das playlists
playlists = []
for playlist in data:
    for video in playlist.get("videos", []):
        playlists.append({
            "Playlist": playlist["playlistName"],
            "Título": video["title"],
            "Autor": video["author"],
            "Duração (s)": video["lengthSeconds"],
            "Publicado em": video["published"],
            "Adicionado em": video["timeAdded"]
        })

# Converter para DataFrame do pandas
df = pd.DataFrame(playlists)

# Salvar como Excel
df.to_excel("playlists.xlsx", index=False)

print("Arquivo playlists.xlsx criado com sucesso! 🚀")