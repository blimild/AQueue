/**
 * @brief aqueue header file
 *
 * @file queue.h
 * @author  wjlee
 * @date    2010-12-17
 *
 */
#ifndef __AQUEUE_H__
#define __AQUEUE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_AQUEUE 10000
#define AQUEUE_DEFAULT_SIZE 8

// general queue structure
typedef struct aqueue {
	void **list;	// 데이터 저장 공간
	int size;		// malloc SIZE
	int first;		// 첫번째 데이터의 위치
	int last;		// 마지막 데이터 + 1의 위치, last == first : full/empty --> 이런 상황을 만들지 않는다 .. 즉 full은 없음
} aqueue;

// aparser queue functions
/*
	aqueue자체는 local 변수라고 가정하고 개발이 되었다.
	- _initialize() : 메모리의 내용을 초기화 한다.
	- _clear() : list에 딸린 내용을 정리하고, 메모리의 내용을 초기화 한다.
		element_free()가 실제로 free할 것인지 아닌지를 결정한다. -- 이것은 사용하는 쪽에서 제공
		list 자체는 free된다.
		이 함수는 반복적으로 호출되는 함수 안에서 aqueue가 사용된 경우 메모리를 모두 반납하게 하기 위해 필요하다.

	이 두가지만으로는 부족하다고 생각되어 아래와 같은 함수들을 추가하였다.
	- _new() : aqueue 자체를 malloc으로 만들어낸다.
	- _free() : aqueue 자체와 딸린 내용을 모두 free 한다.
	- _clean() : _clear() 함수와 다른점은 list자체를 가능하면 재활용하게 한다는 점이다.
		이 함수는 aqueue가 global 영역에 있어서 메모리를 control 할수 있는 경우에 사용한다.
		max_size를 넘는 list는 조정한다.
*/
void aqueue_initialize(aqueue *q);
void aqueue_clear(aqueue *q, void (*element_free)(void *));
void aqueue_clean(aqueue *q, void (*element_free)(void *), int max_size);
void aqueue_free(aqueue *q, void (*element_free)(void *));
aqueue * aqueue_new();
void aqueue_print(aqueue *q, void (*element_print)(void *));

int aqueue_push(aqueue *q, void *p, int pos);	// -1, -2, -3 형태의 위치는 뒤어서부터 넣기
int aqueue_push_uniq(aqueue *q, void *p, int pos, int (* element_compare)(void *, void *));	 // 중복 금지

void *aqueue_check_dup(aqueue *q, void *p, int (* element_compare)(void *, void *)); // unsorted data의 중복을 체크 할 수 있다. .. push에 의해 만들어진 데이터를 대상을 함
int aqueue_search(aqueue *q, void *p, int (* element_compare)(void *, void *)); // sorted data의 위치 확인 .. 아래 insert에 의해 만들어진 데이터를 대상으로 함

int aqueue_insert(aqueue *q, void *p, int (* element_compare)(void *, void *)); // compare 함수에서 정한 정렬된 순서의 위치에 들어 간다.
int aqueue_insert2(aqueue *q, void *p, int (* element_compare)(void *, void *)); // 순서대로, bsearch
int aqueue_insert_uniq(aqueue *q, void *p, int (* element_compare)(void *, void *)); // 순서대로, bsearch, 중복 금지

void *aqueue_pop(aqueue *q, int pos);
void *aqueue_get(aqueue *q, int pos);
int aqueue_size(aqueue *q);
int aqueue_delete(aqueue *q, void *p); // 주어진 p와 동일한 객체를 제거한다.

#ifdef __cplusplus
};
#endif

#endif
