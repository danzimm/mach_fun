#include <stdio.h>
#include <mach/mach.h>
#include <bootstrap_priv.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

void error(int a, int r, const char *str) {
  const char *err = bootstrap_strerror(r);
  fprintf(stderr, "%s (%#02x) %s\n", str, r, err);
  exit(a);
}

bool list_services_and_hookin(mach_port_t bp, const char *name) {
  mach_port_array_t children = NULL;
  name_array_t names = NULL;
  bootstrap_property_array_t props = NULL;
  mach_msg_type_number_t nchild = 0;
  int i = 0, ret;

  ret = bootstrap_lookup_children(bp, &children, &names, &props, &nchild);
  if (ret && ret != BOOTSTRAP_NO_CHILDREN)
    error(1,ret,"bootstrap_lookup_children");
  printf("Got %d children!\n", nchild);
  for (i = 0; i < nchild; i++) {
    printf("Found service: %s\n", names[i]);
    if (name && strcmp(names[i], name) == 0) {
      puts("Found match");
      return true;
    }
  }
  return false;
}

int main(int argc, const char *argv[]) {
  mach_port_t bp = bootstrap_port, tmp = MACH_PORT_NULL;
  while (bp != tmp) {
    if (tmp)
      puts("Looking at parent");
    if (list_services_and_hookin(bp, argv[1]))
      break;
    tmp = bp;
    bootstrap_parent(bp, &bp);
  }
  return 0;
}
