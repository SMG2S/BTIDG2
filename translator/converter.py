import json
import sys
import os

def writeData(out, file):
    #Ecriture
    f = open(file,"w")
    f.write(out)
    f.close()


def loadData(file):
    with open(file) as f:
        data = json.load(f)
    return data

def numberNeuronCumul(data):
    nbNeuronCumul = []
    nbNeuronCumul.append(data[0]["nbNeuron"]/2)
    for i in range(1,len(data)):
        nbNeuronCumul.append(data[i]["nbNeuron"]/2+nbNeuronCumul[i-1])
    for i in range(len(data)):
        nbNeuronCumul.append(data[i]["nbNeuron"]/2+nbNeuronCumul[len(data)+i-1])
    return nbNeuronCumul

def distribNeuronCumul(data):
    repartitionNeuronCumul = []
    for i in range(len(data)*2):
        repartitionNeuronCumul.append([data[i%len(data)]["typeNeuron"][0]["nbNeuron"]/data[i%len(data)]["nbNeuron"]])
        for j in range(1,data[i%len(data)]["nbTypeNeuron"]):
            repartitionNeuronCumul[i].append(data[i%len(data)]["typeNeuron"][j]["nbNeuron"]/data[i%len(data)]["nbNeuron"]+repartitionNeuronCumul[i][j-1])

    return repartitionNeuronCumul

def probaConnection(data):
    sommeNbNeuron = 0
    for i in range(len(data)):
        sommeNbNeuron+=data[i]["nbNeuron"]
    probaConnect = [[] for i in range(len(data)*2)]
    for i in range(len(data)):
        probaConnect[i] = [[] for n in range(data[i]["nbTypeNeuron"])]
        probaConnect[i+len(data)] = [[] for n in range(data[i]["nbTypeNeuron"])]
        for j in range(data[i]["nbTypeNeuron"]):
            probaConnect[i][j] = [0 for n in range(len(data)*2)]
            probaConnect[i+len(data)][j] = [0 for n in range(len(data)*2)]
            for k in range(len(data)):
                probaConnect[i][j][k] = (1-data[i]["connectionOpposite"])*data[i]["distribution"][k]*data[i]["typeNeuron"][j]["nbConnection"]/sommeNbNeuron
                probaConnect[i][j][k+len(data)] = data[i]["connectionOpposite"]*data[i]["distribution"][k]*data[i]["typeNeuron"][j]["nbConnection"]/sommeNbNeuron
                probaConnect[i+len(data)][j][k] = data[i]["connectionOpposite"]*data[i]["distribution"][k]*data[i]["typeNeuron"][j]["nbConnection"]/sommeNbNeuron
                probaConnect[i+len(data)][j][k+len(data)] = (1-data[i]["connectionOpposite"])*data[i]["distribution"][k]*data[i]["typeNeuron"][j]["nbConnection"]/sommeNbNeuron
    return probaConnect

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise Exception("Please provide the configuration json file as: python ./converter.py config.json")
    data = loadData(sys.argv[1])
    nbNeuronCumul = numberNeuronCumul(data)
    distribNeuronCumul = distribNeuronCumul(data)
    probaConnection = probaConnection(data)

    nbPart = len(nbNeuronCumul)
    #getNbPart
    out="int get_nb_part(){\n\treturn "+str(nbPart)+";\n}\n\n"
    #parametreCerveau
    #les choses intéressante commence ici
    #initialisation
    out+="void paramBrain(Brain *Cerveau, long long *n){\n\tint nbTypeNeuronIci,nb_part="+str(nbPart)+";\n"
    out+="\t*n="+str(int(nbNeuronCumul[-1]))+";\n\tBrainPart *brainPart = malloc(sizeof(BrainPart)*nb_part);\n"
    out+="\tlong long *part_cerv = malloc(sizeof(long long)*nb_part);\n"
    out+="\tpart_cerv[0] = 0;\n"
    for i in range(int(nbPart)-1):
        out+="\tpart_cerv["+str(int(i+1))+"] = "+str(int(nbNeuronCumul[i]))+";\n"


    #partie du cerveau
    for i in range(len(nbNeuronCumul)):
        out+="//partie "+str(i)+"\n"
        out+="\tbrainPart["+str(i)+"].nbTypeNeuron = "+str(len(distribNeuronCumul[i]))+";\n"
        #repartitionNeuronCumulee
        out+="\tbrainPart["+str(i)+"].repartitionNeuronCumulee = malloc(sizeof(double)*"+str(len(distribNeuronCumul[i]))+");\n"
        for j in range(len(distribNeuronCumul[i])):
            out+="\tbrainPart["+str(i)+"].repartitionNeuronCumulee["+str(j)+"] = "+str(distribNeuronCumul[i][j])+";\n"
        #probaConnection
        out += "\tbrainPart["+str(i)+"].probaConnection = malloc(sizeof(double)*"+str(len(distribNeuronCumul[i])*nbPart)+");\n"
        for j in range(len(distribNeuronCumul[i])):
            for k in range(nbPart):
                ind = j*nbPart+k
                out+="\tbrainPart["+str(i)+"].probaConnection["+str(ind)+"] = "+str(probaConnection[i][j][k])+";\n"
    #attribution dans Cerveau
    out+="\tCerveau->dimension = *n;\n\tCerveau->nb_part = nb_part;\n\tCerveau->parties_cerveau = part_cerv;\n\tCerveau->brainPart = brainPart;\n}"

    writeData(out, os.path.splitext(sys.argv[1])[0]+".h")
