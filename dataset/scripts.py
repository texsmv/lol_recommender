import numpy as np
import pandas as pd

data = pd.read_excel("2017matchdataOraclesElixir.xlsx")

nombres = data.champion.unique()
print(len(nombres))
index_t = np.argwhere(nombres==' ')
nombres = np.delete(nombres, index_t)
indices = np.array(range(len(nombres)))

print(len(nombres))
dataframe_champions = pd.DataFrame(list(zip(indices, nombres)))
dataframe_champions.to_csv("champions.csv", index = False, header=False)

game_ids = data.gameid.unique()
