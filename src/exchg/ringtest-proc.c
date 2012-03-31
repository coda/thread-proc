#include <exchg/ringlink.h>
#include <exchg/wrapper.h>
#include <util/spawn.h>
#include <util/config.h>

#include <echotwo.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Идея такая: i-тая вершина в дереве отвечает за некоторую часть окружности,
// которая должна быть подцеплена в общий круг. Общий круг в вершине i видится
// через i.downread и i.upwrite. Допустим, что i.downread будет использовано в
// самой вершине i для чтения. Поэтому, допустим, для i.right.downread должна
// быть создана новая pipe. Для i.right должен быть свой upwrite ведущий в
// downread для i.left, upwrite для которой должен оставаться i.upwrite. Хорошо.

// Если i.right не существует, тогда новый downread, создаваемый i, должен
// вести напрямую в downread для i.left.

// Если i.left не существует, тогда i должна использовать read и write
// использовать

// Связь для i-го узла - это uplink. i%2 == 1 - это i.right узел

typedef struct
{
	int downread;
	int upwrite;
} pipelink;

static void pldrop(const pipelink *const pl)
{
	int t;

	if((t = pl->downread) != -1)
	{
		wprclose(t);
	}

	if((t = pl->upwrite) != -1)
	{
		wprclose(t);
	}
}

typedef struct
{
	const treeplugin * tp;
	unsigned id;

	pipelink links[2];
	int readend;
	int writeend;
} ringargument;

enum { right = 0, left }; // left child has 2 * id + 1 number

// О том, что нужно закрывать, чтобы через fork не просочились болтающиеся
// ссылки на каналы.

// 1. Два дескриптора, которые используются в родительском процессе:
//	readend
//	writeend

// 2. Связь с кольцом (uplink) сиблингов

// 3. Для работы не нужны uplink-и, созданные для дочерних процессов,
// их можно закрыть

// 4. Можно освободить память, занимаемую prevarg-ом, после инициализации
// lnk и закрытия свободных концов каналов. Память для arg можно освободить
// после закрытия uplink-ов и инициализации ringlink

static void * makeargument(
	const treeplugin *const tp,
	const unsigned id, const void *const prevarg)
{
	const ringargument *const pra = (const ringargument *)prevarg;
	const runconfig *const rc = tp->rc;

	ringargument *const ra = malloc(sizeof(ringargument));
	if(ra) { } else
	{
		fail("can't allocate ring argument");
	}

	*ra = (ringargument){
		.tp = tp,
		.id = id,
		.links = { { -1, -1 }, { -1, -1 } },
		.readend = -1,
		.writeend = -1 };

	pipelink lnk;

	if(id == 0)
	{
		int fds[2];
		wprpipe(fds);
		lnk.downread = fds[piperead];
		lnk.upwrite = fds[pipewrite];
	}
	else
	{
		lnk = pra->links[id%2];

		wprclose(pra->readend);
		wprclose(pra->writeend);
		pldrop(pra->links + (id%2 ^ 1));
	}

	free((void *)pra);

	ra->readend = lnk.downread;

	if(2 * id + 1 >= rc->nworkers) // leaf node
	{
		ra->writeend = lnk.upwrite;
	}
	else
	{
		ra->links[left].upwrite = lnk.upwrite;

		int fds[2];
		wprpipe(fds);
		ra->writeend = fds[pipewrite];

		if(2 * id + 2 < rc->nworkers)
		{
			ra->links[right].downread = fds[piperead];

			wprpipe(fds);
			ra->links[left].downread = fds[piperead];
			ra->links[right].upwrite = fds[pipewrite];
		}
		else // no subtree to the right
		{
			ra->links[left].downread = fds[piperead];
		}
	}

	return ra;
}

static void dropargument(void *const arg)
{
	const ringargument *const ra = (const ringargument *)arg;

	printf("dropping on %03u fds: %d %d\n",
		ra->id, ra->readend, ra->writeend);

	wprclose(ra->readend);
	wprclose(ra->writeend);
	free(arg);
}

static void treeroutine(const void *const arg)
{
	const ringargument *const ra = (const ringargument *)arg;
	const runconfig *const rc = ra->tp->rc;
	ringlink rl;
	rlform(&rl, ra->readend, ra->writeend);

	pldrop(ra->links + 0);
	pldrop(ra->links + 1);
	free((void *)ra);

	printf("proc %d ready to run\n", getpid());
	sleep(rc->size);

	if(ra->id)
	{
		rlwrite(&rl, rlread(&rl) + 1);
	}
	else
	{
		rlwrite(&rl, 1);
		printf("got at root %u\n", rlread(&rl));
	}
}

int main(const int argc, const char *const argv[])
{
	printf("running with pid %d\n", getpid());

	const treeplugin tp = {
		.rc = formconfig(argc, argv, 64, 0),
		.extra = NULL,
		.treeroutine = treeroutine,
		.makeargument = makeargument,
		.dropargument = dropargument };

	treespawn(&tp);

	sleep(tp.rc->size);

	freeconfig((runconfig *)tp.rc);
	
	return 0;
}
