import numpy as np
import pandas as pd
import csv


def charge_data(filename):
    l = list()
    with open(filename) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for row in csv_reader:
            # print(row[1])
            l.append(row[1])
    return l

def charge_data_(filename):
    l = list()
    with open(filename) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for row in csv_reader:
            l.append(row[1:])
    return l


# def read_file(filename):
#     dic = list()
#     data = pd.read_excel(filename)
#     id_partidas=data["gameid"]
#     result=list(data["result"])
#     id_champions=list(data["champion"])
#     partidas_list = list()
#     partidas_list= [str(tag) for tag in id_partidas]
#     it = 0
#     champions = charge_data("champions.csv")
#     partidas = charge_data("partidas.csv")
#     print("len_partidas: ",len(partidas_list))
#     print("len_champions: ",len(id_champions))
#     for i in range(0,len(partidas_list)):
#         a = tuple()
#         if (id_champions[i] in champions):
#             id_p = partidas.index(partidas_list[i])
#             id_c = champions.index(id_champions[i])
#             r = result[i]
#             a = id_p, id_c, it,r
#             it+=1
#             dic.append(a)
#     for i in range(0,16):
#         print(dic[i])
#     dataframe_ids = pd.DataFrame(dic)
#     dataframe_ids.to_csv("HeroPart.csv", index = False, header=False)
# read_file("2017matchdataOraclesElixir.xlsx")


def read_file(filename):
    dic = list()
    data = pd.read_excel(filename)
    id_partidas=data["gameid"]
    result=list(data["result"])
    id_champions=list(data["champion"])
    servers = data["league"]
    partidas_list =[]
    partidas_list= [str(tag) for tag in id_partidas]
    champions = charge_data("champions.csv")
    partidas = charge_data_("partidas.csv")
    # print(partidas_list[2][0],partidas_list[2][1])
    # id_p = partidas.index([partidas_list[0],servers[0]])
    it = 0;
    # it1 = 0;
    for i in range (0,len(partidas_list)):
        a = tuple()
        if (id_champions[i] in champions):
            id_p = partidas.index([partidas_list[i],servers[i]])
            id_c = champions.index(id_champions[i])
            sr = servers[i]
            r = result[i]
            a = id_p, id_c, it,r,sr
            it+=1
            dic.append(a)
    # print(dic[:20])
        # it1+=1
    dataframe_ids = pd.DataFrame(dic)
    dataframe_ids.to_csv("HeroPart.csv", index = False, header=False)
    
read_file("2017matchdataOraclesElixir.xlsx")




