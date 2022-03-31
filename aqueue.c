#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>

#include <aqueue.h>

// 맨 처음 초기화 할때만 사용한다.
void aqueue_initialize(aqueue *q)
{
	q->list = NULL;

	q->size = 0;
	q->first = 0;
	q->last = 0;
}

// q를 초기화 한다.
// 중간에 지울 필요가 있을 때 한다.
void aqueue_clear(aqueue *q, void (*element_free)(void *))
{
	int i, used_size;

	if(element_free != NULL && !(q->size == 0 || q->first == q->last) )
	{
		used_size = (q->last + q->size - q->first) % q->size;

		for(i = 0; i < used_size; i++)
			element_free(q->list[(q->first + i)%q->size]);
	}

	if(q->list != NULL && q->size != 0)
	{
		free(q->list);
		q->list = NULL;
	}

	q->size = 0;
	q->first = 0;
	q->last = 0;
}

// 가능하면 list를 제활용하도록 한다. 
// 그러나 max_size를 넘는 list를 제한한다.
void aqueue_clean(aqueue *q, void (*element_free)(void *), int max_size)
{
	int i, used_size;

	if(element_free != NULL && !(q->size == 0 || q->first == q->last) )
	{
		used_size = (q->last + q->size - q->first) % q->size;

		for(i = 0; i < used_size; i++)
			element_free(q->list[(q->first + i)%q->size]);
	}

	if(q->list != NULL && q->size != 0)
	{
		if(max_size <= 0)
		{
			free(q->list);
			q->list = NULL;
			q->size = 0;
		}
		else
		{
			while(q->size > max_size)
				q->size >>= 1;

			q->list = (void **) realloc(q->list, sizeof(void *) * q->size);
		}
	}

	q->first = 0;
	q->last = 0;
}

void aqueue_free(aqueue *q, void (*element_free)(void *))
{
	if(q != NULL)
	{
		aqueue_clear(q, element_free);
		free(q);
	}
}

aqueue * aqueue_new()
{
	return (aqueue *)calloc(1, sizeof(aqueue));
}

void aqueue_print(aqueue *q, void (*element_print)(void *))
{
	int i, used_size;

	fprintf(stderr, "q=%p\n", q);
	if(q == NULL) return;

	fprintf(stderr, "\tq->first=%d\n", q->first);
	fprintf(stderr, "\tq->last=%d\n", q->last);
	fprintf(stderr, "\tq->size=%d\n", q->size);

	if(element_print != NULL && !(q->size == 0 || q->first == q->last) )
	{
		used_size = (q->last + q->size - q->first) % q->size;

		for(i = 0; i < used_size; i++)
			element_print(q->list[(q->first + i)%q->size]);
	}
}

// queue는 circular 
// q의 index와 갯수 문제
// l .... f : n-5
// l ... f : n-4
// l .. f : n-3
// l . f : n-2
// l f : n-1 		-- full
// f=l : 0			-- empty
// f l : 1
// f . l : 2
// f .. l : 3
// f ... l : 4
// 데이터 위치는  [f ... ] l 에서 f는 포함하고 l은 포함하지 않는다.

// q가 비어있는가를 체크한다.
int aqueue_size(aqueue *q)
{
	if(q == NULL)
		return -1;

	if(q->size == 0)
		return 0;

	return (q->last + q->size - q->first) % q->size;
}


// q의 pos 위치의 data를 가져온다. -- q에 data는 남아 있음
//  return = NULL if pos is non-data-position 
//
// [0, 1, 2, ... n-1] n개의 데이터의 index
// pos = n-1 과 pos = -1은 같은 의미이다.
//
// data가 a b c d e f g 가 있을때 insert index의 의미 
// 0:a 1:b 2:c 3:d 4:e 5:f 6:g -- 그외의 숫자는 결과 없음으로 처리한다.
// -7:a -6:b -5:c -4:d -3:e -2:f -1:g : 그외의 숫자는 결과 없음으로 처리한다.

void *aqueue_get(aqueue *q, int pos)
{
	int used_size;

	if(q->size == 0 || q->first == q->last)
		return NULL;

	used_size = (q->last + q->size - q->first) % q->size;

	// 위치가 범위를 넘어가면 결과 없음
	if(pos < 0)
		pos += used_size;

	if(pos >= used_size || pos < 0)
		return NULL;

	return q->list[(q->first + pos)%q->size];
}


// q의 pos 위치에 data p를 넣는다.
//  pos = 0:맨앞, -1:맨뒤, n:중간 n번째 위치
//	return = -1 if malloc error occurs
//
// [0, 1, 2, ... n-1] n개의 데이터의 index
// pos = n 과 pos = -1은 같은 의미이다.
//
// data가 a b c d e f g 가 있을때 insert index의 의미 
// 0 a 1 b 2 c 3 d 4 e 5 f 6 g 7 : 그외의 숫자는 -1(맨뒤)로 처리된다.
// -8 a -7 b -6 c -5 d -4 e -3 f -2 g -1 : 그외의 숫자는 0(맨 앞으로 처리된다.
//
int aqueue_push(aqueue *q, void *p, int pos)
{
	int i, used_size;

	// 공간 처음으로 만들기
	if(q->size == 0)
	{
		q->list = (void **) malloc(sizeof(void *) * AQUEUE_DEFAULT_SIZE);
		if(q->list == NULL)
			return -1;
		
		q->size = AQUEUE_DEFAULT_SIZE;
		q->first = 0;
		q->last = 0;
	}
	// queue의 크기 제한, 일단은 제한 없음
//	else if( (q->last + q->size - q->first) % q->size >= MAX_AQUEUE)
//	{
//		return -1;
//	}
	// 기존의 공간이 모자라는 경우.
	else if((q->last + 1)%q->size == q->first)
	{
		// 항상 2배로 키운다.
		q->list = (void **) realloc(q->list, sizeof(void *) * q->size * 2);
		if(q->list == NULL)
			return -1;

		// 0 ... last, first ... end : 이 형태의 순서인 경우
		if(q->last < q->first)
		{
			// 앞부분을 이동 : first ... end 부분을 새로 할당한 공간에 이동한다.
			if((q->size - q->first) < q->last) // if((q->size - q->first) < q->last - 0)
			{
				memcpy(q->list + q->size+q->first, q->list+q->first, (q->size-q->first)*sizeof(void *));
				q->first += q->size;
			} 
			// 뒷부분을 이동 : 0 ... last 부분을 기존의 first ... end에 붙인다.
			else
			{
				memcpy(q->list + q->size, q->list, q->last*sizeof(void *));
				q->last += q->size;
			}
		}
		q->size *= 2;
	}

	used_size = (q->last + q->size - q->first) % q->size;

	// 위치가 범위를 넘어가면 맨 뒤에 추가
	if(pos >= used_size)
		pos = -1;
	if(pos < -1)
	{
		// 음수인 경우 범위를 넘어 가면 맨 앞에 추가
		pos += used_size + 1;
		if(pos < 0)
			pos = 0;
	}

	// 맨 앞에 넣기
	if(pos == 0)
	{
		q->first = (q->first-1+q->size)%q->size; // first는 이동하고 넣기
		q->list[q->first] = p;
	}
	// 맨뒤에 넣기
	else if(pos == -1)
	{
		pos = q->last;
		q->list[q->last] = p;
		q->last = (q->last+1)%q->size; // last는 넣고 이동
	}
	// 중간에 넣기
	else
	{
		// pos가 뒷부분에 더가까운 경우
		if(pos > used_size/2)
		{
			for(i = used_size; i > pos; i--)
				q->list[(q->first + i)%q->size] = q->list[(q->first + i - 1)%q->size];
			q->last = (q->last+1)%q->size;
		}
		// pos가 앞부분에 더가까운 경우
		else
		{
			for(i = -1; i < pos; i++)
				q->list[(q->first + i + q->size)%q->size] = q->list[(q->first + i + 1)%q->size];
			q->first = (q->first-1+q->size)%q->size;
		}
		q->list[(q->first + pos)%q->size] = p;
	}

	return pos;
}

// pos 위치에 넣는데, 중복은 허용하지 않는다.
int aqueue_push_uniq(aqueue *q, void *p, int pos, int (* element_compare)(void *, void *))
{
	void *e;
	int i, used_size;

	used_size = aqueue_size(q);
	for(i = 0; i < used_size; i++)
	{
		e = aqueue_get(q, i);
		if(element_compare(e, p) == 0)
			return 1; // sampe element exist
	}
	return aqueue_push(q, p, pos);
}

// 중복이 있는지를 조사한다.
void *aqueue_check_dup(aqueue *q, void *p, int (* element_compare)(void *, void *))
{
	void *e;
	int i, used_size;

	used_size = aqueue_size(q);
	for(i = 0; i < used_size; i++)
	{
		e = aqueue_get(q, i);
		if(element_compare(e, p) == 0)
			return e; // sampe element exist
	}
	return NULL;
}

/*
return 값의 정의
	r < 0 : 없음
	r >= 0 : 일치하는 데이터의 위치
*/
// 몇번째 위치에 있는지를 알아낸다.
int aqueue_search(aqueue *q, void *p, int (* element_compare)(void *, void *))
{
	void *e;
	int low=0, high;
	int middle;
	int diff;

	high = aqueue_size(q) - 1;
	while(low <= high)
	{
		middle = (low + high) / 2;
		e = aqueue_get(q, middle);
		diff = element_compare(e, p);

		if (diff < 0)
			low = middle + 1;
		else if (diff > 0)
			high = middle - 1;
		else // matched item
			return middle;
	}
	return -1;
}

// 정렬된 순서로 들어가게 된다... 중복 허용 -- 간단한 방법으로 찾음 .. 이거 없애야 하는가?
int aqueue_insert(aqueue *q, void *p, int (* element_compare)(void *, void *))
{
	void *e;
	int i, used_size;

	used_size = aqueue_size(q);
	for(i = 0; i < used_size; i++)
	{
		e = aqueue_get(q, i);
		if(element_compare(e, p) > 0)
			break;
	}
	return aqueue_push(q, p, i);
}

// 정렬된 순서로 들어가게 된다... 중복 허용 -- bsearch 방식으로 빠르게 찾기
int aqueue_insert2(aqueue *q, void *p, int (* element_compare)(void *, void *))
{
	void *e;
	int low=0, high;
	int middle=0;
	int diff;

	high = aqueue_size(q) - 1;
	while(low <= high)
	{
		middle = (low + high) / 2;
		e = aqueue_get(q, middle);
		diff = element_compare(e, p);

		if (diff < 0)
			low = middle + 1;
		else if (diff > 0)
			high = middle - 1;
		else // matched item
			break; // 이경우 중복인데 ... 안전하게 하기 위해서는 aqueue_search를 사용하여 체크를 먼저 하는 것이 좋다.
	}
	// 결과가 없는 경우, 항상 H+1 == L 형태가 된다. 이때 L의 위치에 새로운 데이터를 넣으면 된다.
	if(low > high)
		middle = low;

	return aqueue_push(q, p, middle);
}

// 정렬된 순서로 들어가게 된다... 중복 허용하지 않음
int aqueue_insert_uniq(aqueue *q, void *p, int (* element_compare)(void *, void *))
{
	void *e;
	int low=0, high;
	int middle=0;
	int diff;

	high = aqueue_size(q) - 1;
	while(low <= high)
	{
		middle = (low + high) / 2;
		e = aqueue_get(q, middle);
		diff = element_compare(e, p);

		if (diff < 0)
			low = middle + 1;
		else if (diff > 0)
			high = middle - 1;
		else // matched item
			return middle;
	}
	// 결과가 없는 경우, 항상 H+1 == L 형태가 된다. 이때 L의 위치에 새로운 데이터를 넣으면 된다.
	if(low > high)
		middle = low;

	return aqueue_push(q, p, middle);
}

// q의 pos 위치의 data를 꺼낸다. -- q에서 제거됨
//  pos = 0, -1, n
//  return = NULL if pos is non-data-position 
//
//  데이터를 꺼내는 위치(pos)는 aqueue_get()과 동일하다.
void *aqueue_pop(aqueue *q, int pos)
{
	void *ptr;
	int i, used_size;

	if(q->size == 0 || q->first == q->last)
		return NULL;

	used_size = (q->last + q->size - q->first) % q->size;

	// 위치가 범위를 넘어가면 결과 없음
	if(pos < 0)
		pos += used_size;

	if(pos >= used_size || pos < 0)
		return NULL;

	// 맨앞 맨뒤의 경우 일반화 가능하지만 ... 여기서는 이해를 쉽게 하기 위해 별도로 둔다.
	if(pos == 0)
	{
		ptr = q->list[q->first];
		q->first = (q->first + 1)%q->size;
	}
	else if(pos == -1)
	{
		q->last = (q->last + q->size - 1)%q->size;
		ptr = q->list[q->last];
	}
	else
	{
		ptr = q->list[(q->first + pos)%q->size];

		// pos가 뒷부분에 더가까운 경우
		if(pos > used_size/2)
		{
			for(i = pos; i < used_size-1; i++)
				q->list[(q->first + i)%q->size] = q->list[(q->first + i + 1)%q->size];
			q->last = (q->last-1+q->size)%q->size;
		}
		// pos가 앞부분에 더가까운 경우
		else
		{
			for(i = pos; i > 0; i--)
				q->list[(q->first + i)%q->size] = q->list[(q->first + i - 1)%q->size];
			q->first = (q->first+1)%q->size;
		}
	}

	return ptr;
}

int aqueue_delete(aqueue *q, void *p)
{
	int qsize;
	int i;
	void *e;

	qsize = aqueue_size(q);
	for(i = 0; i < qsize; i++)
	{
		e = aqueue_get(q, i);
		if(e == p)
		{
			aqueue_pop(q, i);
			return 1;
		}
	}
	return 0;
}

