#include "sortvec.h" //includes stdlib.h
#include <assert.h>  //assert();
#include <stdbool.h> // true, false
#include <string.h>  //memmove()

/*Сокрытая реализация типа данных*/
typedef struct _SortedVec {
	long max_size;	  // Максимальная вместимость набора
	long cur_size;	  // Текущий размер набора
	DATATYPE * data;  // Область памяти
	DATATYPE * begin; // Указатель на первый действительный элемент
} SortedVec;

/*Сортировка/поиск элементов по возрастанию*/
static int SortedVecComparFunc_(void const * a, void const * b)
{
	int const num1 = (*(DATATYPE *)a)->access_count;
	int const num2 = (*(DATATYPE *)b)->access_count;
	return (num1 > num2) - (num2 > num1);
}

/*Возврат true при успехе, false при ошибке выделения памяти*/
static bool SortedVecResize_(SortedVec * const this, long const newsize)
{
	if (this->max_size >= newsize)
		return true;

	/*Перевыделение памяти, её проверка*/
	DATATYPE * const newdata = (DATATYPE *)realloc(
		this->data, (size_t)(newsize * (long)sizeof(DATATYPE)));
	if (newdata == NULL)
		return false;

	// Расстояние указателя на данные от указателя начала памяти
	long const BeginFromDataDiff = (long)(this->begin - this->data);
	/*Перестановка раземеров, указателей*/
	this->max_size = newsize;
	this->data = newdata;
	this->begin = this->data + BeginFromDataDiff;
	return true;
}

#define DEFAULT_SIZE 100 // Размер инита по умолчанию

SortedVec * SortedVecInit(void)
{
	// Выделение памяти под дескриптор
	SortedVec * const ptr = (SortedVec *)malloc(sizeof(SortedVec));
	if (!ptr) // проверка памяти
		return NULL;
	// выделение памяти под данные
	ptr->data = (DATATYPE *)calloc(DEFAULT_SIZE, sizeof(DATATYPE));
	if (!ptr->data) { // проверка памяти
		free(ptr);
		return NULL;
	}
	// Задание максимального размера
	ptr->max_size = DEFAULT_SIZE;
	// Задание текущего нулевого размера
	ptr->cur_size = 0;
	/*Если вектор пустой, обозначается начало посреди области памяти*/
	ptr->begin = ptr->data + ptr->max_size / 2;

	// Возврат памяти
	return ptr;
}

// Очистка памяти
void SortedVecDeInit(SortedVec ** const ptr)
{
	// Освобождение памяти
	free((*ptr)->data);
	free(*ptr);
	// Перестановка указателя на NULL
	*ptr = NULL;
}

/*Функция проверки упорядоченности вектора*/
/*
static void SortedVecCheckOrder(SortedVec *const this) {
	//Если размер вектора 0 или 1, то проверять нечего
	if (this->cur_size == 1 || this->cur_size == 0)
		return;

	// Перебор массива до конца
	for (long i = 1; i < this->cur_size; ++i) {
		// Если хоть один элемент оказался не на своём месте,
		// вектор нужно пересортировать
		if (this->begin[i - 1]->access_count >
this->begin[i]->access_count) { qsort(this->begin, (size_t)this->cur_size,
			      sizeof(DATATYPE), SortedVecComparFunc_);
			// После принудительного сорта, её проверять не нужно
			return;
		}
	}
}
*/

/* Поиск места для вставки очередного элемента. Возвращает индекс первого
 * элемента, большего или равного данному */
static size_t SortedVecFindPosition_(SortedVec const * const this,
				     DATATYPE const Element)
{
	long i;
	/* Цикл проходится по области действительных данных.
	 * Если он находит значение, большее элемента, цикл
	 * прерывается заранее, в противном случае цикл вернёт
	 * индекс за пределами действительных значений.
	 * Если первый же элемент оказался больше вставляемого,
	 * осуществляется возврат этого индекса*/
	for (i = 0; i < this->cur_size; i++) {
		// if (this->begin[i]->access_count >= Element->access_count)
		if (SortedVecComparFunc_(&(this->begin[i]),
					 &Element))
			break;
	}
	return (size_t)i;
}

// Передвижение вектора от начала выделенной памяти
static void SortedVecMoveFromEnd_(SortedVec * const this)
{
	// Число свободных ячеек
	size_t const FreeSpace = (size_t)(this->max_size - this->cur_size);
	// Сохранение указателя на старые данные
	DATATYPE * const oldptr = this->begin;
	// Перестановка указателя на новое начало
	this->begin = this->data + (FreeSpace / 2);
	// Копирование в новое место из начала
	memmove(this->begin, oldptr, sizeof(DATATYPE) * (size_t)this->cur_size);

	assert(this->begin + this->cur_size < this->data + this->max_size);
}

// Передвижение вектора от начала выделенной памяти
static void SortedVecMoveFromBegin_(SortedVec * const this)
{
	// Число свободных ячеек
	size_t FreeSpace = (size_t)(this->max_size - this->cur_size);
	assert(FreeSpace > 0);

	/* Чтобы при делении на 2 получалось 1, и вектор сдвигался на 1
	элемент */
	if (FreeSpace == 1)
		FreeSpace = 2;

	// Перестановка указателя на новое место
	this->begin = this->data + (FreeSpace / 2);
	// Копирование в новое место из начала
	memmove(this->begin, this->data,
		sizeof(DATATYPE) * (size_t)this->cur_size);

	assert(this->begin > this->data);
}

// Внутренняя функция вставки элемента в массив достаточного размера
static void SortedVecInsert_(SortedVec * const this, DATATYPE const Element)
{
	// Индекс, куда осуществлять вставку
	size_t const pos = SortedVecFindPosition_(this, Element);
	assert(pos <= (size_t)this->cur_size);

	// Если вставка в первой половине
	if (pos < (size_t)this->cur_size / 2) {
		// Если область данных достигла начала выделенной памяти
		if (this->begin == this->data)
			SortedVecMoveFromBegin_(this);

		// Число перемещаемых элементов
		size_t const moveData = pos;
		// Перемещение элементов на один назад
		memmove(this->begin - 1, this->begin,
			moveData * sizeof(DATATYPE));

		// Перемещение указателя на начало вектора на шаг назад
		this->begin--;

	} else { // Вставка во второй половине
		// Если область данных достигла конца выделенной памяти
		if (this->begin + this->cur_size >= this->data + this->max_size)
			SortedVecMoveFromEnd_(this);

		// Число перемещаемых элементов
		size_t const moveData = (size_t)this->cur_size - pos;
		// Перемещение элементов на один вперёд
		memmove(this->begin + pos + 1, this->begin + pos,
			moveData * sizeof(DATATYPE));
	}

	assert(this->begin + pos < this->data + this->max_size);
	assert(this->begin >= this->data);
	assert(this->begin + pos >= this->data);

	// Вставка элемента на его новое место, увеличение счётчика
	this->begin[pos] = Element;
	this->cur_size++;
	return;
}

enum ErrorCode SortedVecRemoveElement(SortedVec * const this,
				      DATATYPE const Element)
{
	// Индекс, куда осуществлять вставку
	size_t const pos = SortedVecFindPosition_(this, Element);
	assert(pos <= (size_t)this->cur_size);

	// Если данного элемента не было, возвращается соответствующая ошибка
	if (pos >= (size_t)this->cur_size)
		return ERR_NOSUCHELEMENT;

	// Если элемент был в первой половине
	if (pos < (size_t)this->cur_size / 2) {
		// Число перемещаемых элементов
		size_t const moveData = pos;
		assert(moveData < (size_t)this->cur_size);

		// Перемещение элементов на один вперёд
		memmove(this->begin + 1, this->begin,
			moveData * sizeof(DATATYPE));

		// Перемещение указателя на начало вектора на шаг вперёд
		this->begin++;

	} else { // Удаление из второй половины
		// Число перемещаемых элементов
		size_t const moveData = (size_t)this->cur_size - pos - 1;
		assert(moveData < (size_t)this->cur_size);

		// Перемещение элементов на один вперёд
		memmove(this->begin + pos, this->begin + pos + 1,
			moveData * sizeof(DATATYPE));
	}

	assert(this->begin + pos < this->data + this->max_size);
	assert(this->begin >= this->data);
	assert(this->begin + pos >= this->data);

	// Вставка элемента на его новое место, увеличение счётчика
	this->cur_size--;
	return ERR_NO;
}

enum ErrorCode SortedVecInsertArray(SortedVec * const this,
				    size_t const ArrSize,
				    DATATYPE const * const Array)
{
	/*Если места в массиве не было достаточно, а расширение не удалось,
	 * возвращается соответствующий код ошибки*/
	if (!SortedVecResize_(this, this->cur_size + (long)ArrSize))
		return ERR_MALLOC;

	/*Перебор массива, вставка каждого элемента*/
	for (size_t i = 0; i < ArrSize; ++i)
		SortedVecInsert_(this, Array[i]);
	return ERR_NO; // Возврат кода отсутствия ошибок
}

enum ErrorCode
SortedVecGet(SortedVec const * const this, long index, DATATYPE * const ptr)
{
	// Если запрошен отрицательный индекс, вектор обходится с конца
	if (index < 0)
		index = this->cur_size + index;

	if (index < 0 || index >= this->cur_size)
		return ERR_NOSUCHELEMENT;

	*ptr = this->begin[index];
	return ERR_NO;
}

long SortedVecSize(SortedVec const * const this)
{
	return this->cur_size;
}

enum ErrorCode SortedVecAddToThis(SortedVec * const this,
				  SortedVec const * const other)
{
	if (!SortedVecInsertArray(this, (size_t)(other->cur_size),
				  other->begin))
		return ERR_MALLOC; /*Возврат ошибки памяти при неудаче
				      выделения*/
	return ERR_NO;		   // Возврат кода отсутствия ошибок при успехе
}

// Максимальный элемент вектора
enum ErrorCode SortedVecGetMax(SortedVec const * const this,
			       DATATYPE * const ptr)
{
	return SortedVecGet(this, this->cur_size - 1, ptr);
}

// Минимальный элемент вектора вектора
enum ErrorCode SortedVecGetMin(SortedVec const * const this,
			       DATATYPE * const ptr)
{
	return SortedVecGet(this, 0, ptr);
}

int SortedVecDoesBelong(SortedVec const * const this, DATATYPE const element)
{
	return ((DATATYPE *)bsearch(&element, this->begin,
				    (size_t)(this->cur_size), sizeof(DATATYPE),
				    SortedVecComparFunc_))
		       ? 1
		       : 0;
}

/*Вставка одного элемента по указателю вектор*/
enum ErrorCode SortedVecInsert(SortedVec * const vec, DATATYPE const * ptr)
{
	return SortedVecInsertArray(vec, 1, ptr);
}

DATATYPE * SortedVecGetBegin(SortedVec const * vec)
{
	return vec->begin;
}
