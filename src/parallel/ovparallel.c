#include "ovutil/parallel.h"

#include "ovthreads.h"

struct ovparallel {
  mtx_t mtx;
  struct cndvar cv;
  struct cndvar cv2;

  size_t threads;
  thrd_t *thrd;

  size_t n;
  ovparallel_task task;
  void *userdata;
};

static int worker(void *userdata) {
  struct ovparallel *const p = userdata;
  for (;;) {
    cndvar_lock(&p->cv);
    cndvar_wait_while(&p->cv, 0);
    int const v = --p->cv.var;
    cndvar_unlock(&p->cv);

    if (v < 0) {
      break;
    }

    p->task(p->userdata, (size_t)v, p->n);

    cndvar_lock(&p->cv2);
    if (++p->cv2.var == (int)p->n) {
      cnd_signal(&p->cv2.cnd);
    }
    cndvar_unlock(&p->cv2);
  }
  return 0;
}

NODISCARD error ovparallel_create(struct ovparallel **pp, size_t const threads) {
  if (!pp || *pp) {
    return errg(err_invalid_arugment);
  }
  error err = eok();
  struct ovparallel *p = NULL;
  size_t started = 0;

  size_t num_threads = threads;
  if (num_threads == 0) {
    err = get_number_of_physical_cores(&num_threads);
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
  }

  err = mem(pp, 1, sizeof(struct ovparallel));
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  p = *pp;
  *p = (struct ovparallel){
      .threads = num_threads,
  };
  mtx_init(&p->mtx, mtx_plain | mtx_recursive);
  cndvar_init(&p->cv);
  cndvar_init(&p->cv2);
  err = mem(&p->thrd, num_threads, sizeof(thrd_t));
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  for (size_t i = 0; i < num_threads; ++i) {
    int r = thrd_create(p->thrd + i, worker, p);
    if (r != thrd_success) {
      err = errg(r == thrd_nomem ? err_out_of_memory : err_unexpected);
      goto cleanup;
    }
    ++started;
  }
cleanup:
  if (efailed(err)) {
    if (p) {
      cndvar_lock(&p->cv);
      cndvar_broadcast(&p->cv, -1);
      cndvar_unlock(&p->cv);
      for (size_t i = 0; i < started; ++i) {
        thrd_join(p->thrd[i], NULL);
      }
      ereport(mem_free(&p->thrd));
      cndvar_exit(&p->cv2);
      cndvar_exit(&p->cv);
      mtx_destroy(&p->mtx);
      ereport(mem_free(pp));
    }
  }
  return err;
}

NODISCARD error ovparallel_destroy(struct ovparallel **const pp) {
  if (!pp || !*pp) {
    return errg(err_invalid_arugment);
  }
  struct ovparallel *p = *pp;
  mtx_lock(&p->mtx);
  cndvar_lock(&p->cv);
  cndvar_broadcast(&p->cv, -1);
  cndvar_unlock(&p->cv);
  for (size_t i = 0; i < p->threads; ++i) {
    thrd_join(p->thrd[i], NULL);
  }
  ereport(mem_free(&p->thrd));
  cndvar_exit(&p->cv2);
  cndvar_exit(&p->cv);
  mtx_destroy(&p->mtx);
  ereport(mem_free(pp));
  return eok();
}

void ovparallel_for(struct ovparallel *const p, ovparallel_task fn, void *const userdata, size_t const n) {
  mtx_lock(&p->mtx);

  p->n = n == 0 ? p->threads : n;
  p->task = fn;
  p->userdata = userdata;

  cndvar_lock(&p->cv);
  cndvar_broadcast(&p->cv, (int)p->n);
  cndvar_unlock(&p->cv);

  cndvar_lock(&p->cv2);
  cndvar_wait_until(&p->cv2, (int)p->n);
  p->cv2.var = 0;
  cndvar_unlock(&p->cv2);

  mtx_unlock(&p->mtx);
}
