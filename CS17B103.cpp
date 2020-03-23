#include <iostream>
#include <fstream>
#include <set>
using namespace std;

class Block
{
public:
	int tag;
	bool valid;
	bool dirty;
	int last_access;

	Block()
	{
		last_access = 0;
		tag = 0;
		valid = 0;
		dirty = 0;
	}
};

class Cache
{
	Block **B;
	public:
	int size; 		
	int b_size;
	int ast;
	int rep;
	int sets;
	int ways;
	set <int, greater<int>> *tag_table;

	//to output the following counts
	int accesssCount;
	int readCount;
	int writeCount;
	int missCount;
	int compulsoryMiss;
	int capacityMiss;
	int conflictMiss;
	int readMiss;
	int writeMiss;
	int dirtyEvicCount;
	Cache()
	{
		size = 0;
		b_size = 0;
		sets = 0;
		ways = 0;
		ast = 0;
		rep = 0;
		B = NULL;

		accesssCount = 0;
		readCount = 0;
		writeCount = 0;
		missCount = 0;
		compulsoryMiss = 0;
		capacityMiss = 0;
		conflictMiss = 0;
		readMiss = 0;
		writeMiss = 0;
		dirtyEvicCount = 0;		
	}
	void initialize(int a, int b, int c, int d)
	{
		size = a;
		b_size = b;
		ast = c;
		rep = d;
		
		if(ast == 0)		//fully associative
		{	
			sets = 1;
			ways = size/b_size;
		}		
		else						//ast way set associative
		{	
			sets = (size/b_size)/ast;
			ways  = ast;
		}
		B = new Block*[sets];
		for(int i=0;i<sets;i++)
			B[i] = new Block[ways];
		
		tag_table = new set<int, greater<int>>[sets];
	}

	void instuction(long long int ins)
	{
		accesssCount++;
		int rw, bo, nSet, currTag;

		long long int inst = ins & (0x7fffffff);
		if(ins == inst)
			rw=0;
		else rw=1;

		inst /= b_size;
		nSet = inst%sets;
		inst = inst/sets;
		currTag = inst;

		if(rw)
			writeCount++;
		else
			readCount++;		
		readwrite(currTag, nSet, rw);
	}

	void readwrite(int currTag, int nSet, int rw)
	{
		int i, j;
		if(tag_table[nSet].count(currTag))
		{
			for(i=0; i<ways; i++)
			{
				if(B[nSet][i].tag == currTag)	//found
				{
					B[nSet][i].last_access = 0;
					if(rw == 1)
						B[nSet][i].dirty = 1;
					for(j=0;j<ways;j++)			//change last access time
					{
						if(B[nSet][j].valid)
							B[nSet][j].last_access++;
					}
					break;
				}
			}

			if(i == ways)
			{
				missCount++;
				if(rw)
					writeMiss++;
				else 
					readMiss++;
				//replacement policy gives j
				j = replacement(nSet);
				if(B[nSet][j].dirty == 1)
					dirtyEvicCount++;

				B[nSet][j].tag = currTag;
				B[nSet][j].valid = 1 ;
				B[nSet][i].last_access = 0;
				if(rw == 1)
					B[nSet][i].dirty = 1;

				for(j=0;j<ways;j++)			//change last access time
				{
					if(B[nSet][j].valid)
						B[nSet][j].last_access++;
				}
			}
		}

		else		//tag absent => not present in cache
		{
			missCount++;
			if(rw)
				writeMiss++;
			else 
				readMiss++;

			tag_table[nSet].insert(currTag);
			compulsoryMiss++;
			for(i=0; i<ways; i++)
			{
				if(B[nSet][i].valid == 0)		//empty block socket found
				{
					B[nSet][i].valid = 1;
					B[nSet][i].tag = currTag;
					B[nSet][i].last_access = 0;
					if(rw == 1)
						B[nSet][i].dirty = 1;
					for(j=0;j<ways;j++)			//change last access time
					{
						if(B[nSet][j].valid)
							B[nSet][j].last_access++;
					}
					break;
				}
			}

			if(i == ways)						//full
			{
				//replacement policy gives "j"
				j = replacement(nSet);
				if(B[nSet][j].dirty == 1)
					dirtyEvicCount++;

				B[nSet][j].tag = currTag;
				B[nSet][j].valid = 1 ;
				B[nSet][i].last_access = 0;
				if(rw == 1)
					B[nSet][i].dirty = 1;
				for(j=0;j<ways;j++)			//change last access time
				{
					if(B[nSet][j].valid)
						B[nSet][j].last_access++;
				}
			}
		}
	}

	int replacement(int nSet)
	{
		int i, ans, index;
		if(rep == 0)	//random
		{
			index = rand()%ways;
			return index;
		}
		if(rep > 0)	//LRU
		{	
			ans = -1;
			index = 0;
			for(i=0; i<ways; i++)
			{
				if(ans<B[nSet][i].last_access)
				{
					ans = B[nSet][i].last_access;
					index = i;
				}
			}
			return index;
		}
	}
};


int main()
{
	int a, b, c, d;
	ifstream fin;
	ofstream fout;
	fout.open("output.txt");
	fin.open("input.txt");
	long long int temp;
	fin  >> a >> b >> c >> d;
	Cache oneGig;
	oneGig.initialize(a, b, c, d);
	while(true)
	{
		fin >> hex >> temp;
		if(fin.eof()) break;
		oneGig.instuction(temp);
	}
	if(oneGig.sets>1)
	{
		oneGig.capacityMiss = 0;
		oneGig.conflictMiss = oneGig.missCount - oneGig.compulsoryMiss;
	}
	else
	{
		oneGig.capacityMiss = oneGig.missCount - oneGig.compulsoryMiss;
		oneGig.conflictMiss = 0;
	}

	fout << oneGig.size << endl;
	fout << oneGig.b_size << endl;
	fout << oneGig.ast << endl;
	fout << oneGig.rep << endl;
	fout << oneGig.accesssCount << endl;
	fout << oneGig.readCount << endl;
	fout << oneGig.writeCount << endl;
	fout << oneGig.missCount << endl;
	fout << oneGig.compulsoryMiss << endl;
	fout << oneGig.conflictMiss << endl;
	fout << oneGig.capacityMiss << endl;
	fout << oneGig.readMiss << endl;
	fout << oneGig.writeMiss << endl;
	fout << oneGig.dirtyEvicCount << endl;
	return 0;
}
