void destructeurBrain(Brain *Cerveau){
	free(Cerveau->brainPart[0].repartitionNeuronCumulee);
	free(Cerveau->brainPart[0].probaConnection);
	free(Cerveau->brainPart[1].repartitionNeuronCumulee);
	free(Cerveau->brainPart[1].probaConnection);
	free(Cerveau->brainPart[2].repartitionNeuronCumulee);
	free(Cerveau->brainPart[2].probaConnection);
	free(Cerveau->brainPart[3].repartitionNeuronCumulee);
	free(Cerveau->brainPart[3].probaConnection);
	free(Cerveau->parties_cerveau);
	free(Cerveau->brainPart);
}

void paramBrain(Brain *Cerveau, long long *n){
	int nbTypeNeuronIci,nb_part=4;
	*n=15;
	BrainPart *brainPart = malloc(sizeof(BrainPart)*nb_part);
	long long *part_cerv = malloc(sizeof(long long)*nb_part);
	part_cerv[0] = 0;
	part_cerv[1] = 5;
	part_cerv[2] = 7;
	part_cerv[3] = 12;
//partie 0
	brainPart[0].nbTypeNeuron = 2;
	brainPart[0].repartitionNeuronCumulee = malloc(sizeof(double)*2);
	brainPart[0].repartitionNeuronCumulee[0] = 0.7;
	brainPart[0].repartitionNeuronCumulee[1] = 1.0;
	brainPart[0].probaConnection = malloc(sizeof(double)*8);
	brainPart[0].probaConnection[0] = 0.08533333333333334;
	brainPart[0].probaConnection[1] = 0.021333333333333336;
	brainPart[0].probaConnection[2] = 0.021333333333333336;
	brainPart[0].probaConnection[3] = 0.005333333333333334;
	brainPart[0].probaConnection[4] = 0.21333333333333337;
	brainPart[0].probaConnection[5] = 0.053333333333333344;
	brainPart[0].probaConnection[6] = 0.053333333333333344;
	brainPart[0].probaConnection[7] = 0.013333333333333336;
//partie 1
	brainPart[1].nbTypeNeuron = 1;
	brainPart[1].repartitionNeuronCumulee = malloc(sizeof(double)*1);
	brainPart[1].repartitionNeuronCumulee[0] = 1.0;
	brainPart[1].probaConnection = malloc(sizeof(double)*4);
	brainPart[1].probaConnection[0] = 0.08000000000000002;
	brainPart[1].probaConnection[1] = 0.24000000000000005;
	brainPart[1].probaConnection[2] = 0.020000000000000004;
	brainPart[1].probaConnection[3] = 0.06000000000000001;
//partie 2
	brainPart[2].nbTypeNeuron = 2;
	brainPart[2].repartitionNeuronCumulee = malloc(sizeof(double)*2);
	brainPart[2].repartitionNeuronCumulee[0] = 0.7;
	brainPart[2].repartitionNeuronCumulee[1] = 1.0;
	brainPart[2].probaConnection = malloc(sizeof(double)*8);
	brainPart[2].probaConnection[0] = 0.021333333333333336;
	brainPart[2].probaConnection[1] = 0.005333333333333334;
	brainPart[2].probaConnection[2] = 0.08533333333333334;
	brainPart[2].probaConnection[3] = 0.021333333333333336;
	brainPart[2].probaConnection[4] = 0.053333333333333344;
	brainPart[2].probaConnection[5] = 0.013333333333333336;
	brainPart[2].probaConnection[6] = 0.21333333333333337;
	brainPart[2].probaConnection[7] = 0.053333333333333344;
//partie 3
	brainPart[3].nbTypeNeuron = 1;
	brainPart[3].repartitionNeuronCumulee = malloc(sizeof(double)*1);
	brainPart[3].repartitionNeuronCumulee[0] = 1.0;
	brainPart[3].probaConnection = malloc(sizeof(double)*4);
	brainPart[3].probaConnection[0] = 0.020000000000000004;
	brainPart[3].probaConnection[1] = 0.06000000000000001;
	brainPart[3].probaConnection[2] = 0.08000000000000002;
	brainPart[3].probaConnection[3] = 0.24000000000000005;
	Cerveau->dimension = *n;
	Cerveau->nb_part = nb_part;
	Cerveau->parties_cerveau = part_cerv;
	Cerveau->brainPart = brainPart;
}