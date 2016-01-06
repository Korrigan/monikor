#include <stdlib.h>

#include "metric.h"


void monikor_metric_list_init(monikor_metric_list_t *list) {
  list->first = NULL;
  list->last = NULL;
  list->size = 0;
}


monikor_metric_list_t *monikor_metric_list_new(void) {
  monikor_metric_list_t *list;

  if (!(list = malloc(sizeof(*list))))
    return NULL;
  monikor_metric_list_init(list);
  return list;
}


monikor_metric_list_node_t *monikor_metric_list_node_new(monikor_metric_t *metric) {
  monikor_metric_list_node_t *node;

  if (!(node = malloc(sizeof(*node))))
    return NULL;
  node->next = NULL;
  node->metric = metric;
  return node;
}


void monikor_metric_list_empty(monikor_metric_list_t *list) {
  monikor_metric_list_node_t *cur;
  monikor_metric_list_node_t *next;

  if (!list)
    return;
  cur = list->first;
  while (cur) {
    next = cur->next;
    monikor_metric_list_node_free(cur);
    cur = next;
  }
  list->size = 0;
}

void monikor_metric_list_free(monikor_metric_list_t *list) {
  if (!list)
    return;
  monikor_metric_list_empty(list);
  free(list);
}


void monikor_metric_list_node_free(monikor_metric_list_node_t *node) {
  monikor_metric_free(node->metric);
  free(node);
}


int monikor_metric_list_push(monikor_metric_list_t *list, monikor_metric_t *metric) {
  monikor_metric_list_node_t *node;

  if (!(node = monikor_metric_list_node_new(metric)))
    return -1;
  return monikor_metric_list_push_node(list, node);
}


int monikor_metric_list_push_node(monikor_metric_list_t *list, monikor_metric_list_node_t *node) {
  if (!list->size) {
    list->first = node;
    list->last = node;
  } else {
    list->last->next = node;
    list->last = node;
  }
  list->size++;
  return 0;
}

monikor_metric_t *monikor_metric_list_pop(monikor_metric_list_t *list) {
  monikor_metric_list_node_t *node;
  monikor_metric_t *metric;

  if (!list->first)
    return NULL;
  node = list->first;
  metric = node->metric;
  list->first = node->next;
  if (node == list->last)
    list->last = NULL;
  list->size--;
  free(node);
  return metric;
}

void monikor_metric_list_apply(monikor_metric_list_t *list, void (*apply)(monikor_metric_t *)) {
  for (monikor_metric_list_node_t *cur = list->first; cur; cur = cur->next)
    apply(cur->metric);
}

/*
** This function appends 2 lists
** Please note that tail contents are definitely moved to head, so tail is an empty list after
** the call to monikor_metric_list_concat and can be safely freed with a call to
** monikor_metric_list_delete
*/
void monikor_metric_list_concat(monikor_metric_list_t *head, monikor_metric_list_t *tail) {
  if (head->size && tail->size) {
    head->last->next = tail->first;
    head->last = tail->last;
  } else if (tail->size) {
    head->first = tail->first;
    head->last = tail->last;
  }
  head->size += tail->size;
  monikor_metric_list_init(tail);
}
