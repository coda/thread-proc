#include <exchg/ringlink.h>
#include <exchg/wrapper.h>
#include <util/spawn.h>
#include <util/echotwo.h>
#include <util/config.h>
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

typedef struct
{
	const treeplugin * tp;
	unsigned id;

	pipelink links[2];
	int readend;
	int writeend;
} ringargument;

enum { right = 0, left }; // left child has 2 * id + 1 number

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

	ra->tp = tp;
	ra->id = id;

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
	}

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
	ringlink rl;
	rlform(&rl, ra->readend, ra->writeend);

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
