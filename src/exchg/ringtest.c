#include <exchg/ringlink.h>
#include <util/spawn.h>
#include <util/echotwo.h>
#include <util/config.h>
#include <stdlib.h>

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

static void * makeargument(
	const treeplugin *const tp,
	const unsigned id, const void *const prevarg)
{
	return NULL;
}

static void dropargument(void *const arg)
{
}

static void treeroutine(const void *const arg)
{
}

int main(const int argc, const char *const argv[])
{
	const treeplugin tp = {
		.rc = formconfig(argc, argv, 64, 0),
		.extra = NULL,
		.treeroutine = treeroutine,
		.makeargument = makeargument,
		.dropargument = dropargument };

	treespawn(&tp);

	freeconfig((runconfig *)tp.rc);
	
	return 0;
}
