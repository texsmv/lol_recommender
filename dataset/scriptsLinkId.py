import numpy as np
import pandas as pd


def read_file(filename, filenameOut):
    dic = list()
    data = pd.read_excel(filename)
    ids = data["gameid"]
    servers = data["league"]
    id_temp = ""
    count = 0
    j = 0
    for i in zip(ids,servers):
        a=tuple()
        if(id_temp == ""):
            id_temp = i[0]
            server_tmp =i[1]
        else:
            if(id_temp != i[0] or count > 19):
                a = j,id_temp,server_tmp
                dic.append(a)
                count = 0
                j+=1
                id_temp = i[0]
                server_tmp =i[1]
        count +=1
    a = j,id_temp,server_tmp
    dic.append(a)
    dataframe_ids = pd.DataFrame(dic)
    dataframe_ids.to_csv(filenameOut, index = False, header=False)
    print(j)

read_file("2017matchdataOraclesElixir.xlsx","partidas.csv")
# read_file("taste.xlsx","partidas.csv")

