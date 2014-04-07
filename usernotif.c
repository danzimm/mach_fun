#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <servers/bootstrap.h>
#include <bootstrap_priv.h>

// below is just kinda a make shift dictionary creator... mostly for my own practice, not meant to be too practical
// {{{

typedef struct dict {
  char **keys;
  char **values;
  size_t npairs;
} dict_t;

dict_t *new_dict(void) {
  dict_t *ret = malloc(sizeof(dict_t));
  bzero(ret, sizeof(dict_t));
  return ret;
}

void dict_add_pair(dict_t *dict, char *key, char *val) {
  if (!dict)
    return;
  dict->keys = reallocf(dict->keys, sizeof(char *) *(++dict->npairs));
  dict->values = reallocf(dict->values, sizeof(char *) *(dict->npairs));
  char *kcopy = malloc(strlen(key)+1);
  char *vcopy = malloc(strlen(val)+1);
  strcpy(kcopy, key);
  strcpy(vcopy, val);
  dict->keys[dict->npairs-1] = kcopy;
  dict->values[dict->npairs-1] = vcopy;
}

void dict_release(dict_t *dict) {
  if (!dict)
    return;
  size_t i;
  for (i = 0; i < dict->npairs; i++) {
    free(dict->keys[i]);
    free(dict->values[i]);
  }
  free(dict->keys);
  free(dict->values);
  free(dict);
}

/*
 
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>cat</key>
	<string>meow</string>
</dict>
</plist>

*/

char *dict_to_xml(dict_t *dict) {
  const char *header = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n<plist version=\"1.0\">\n<dict>\n";
  const char *suffix = "</dict>\n</plist>\n";
  const char *keyb = "\t<key>";
  const char *keya = "</key>\n";
  const char *valb = "\t<string>";
  const char *vala = "</string>\n";
  size_t i;
  char *ret = malloc(strlen(header)+1);
  strcpy(ret, header);
  
  for (i = 0; i < dict->npairs; i++) {
    char *key = dict->keys[i];
    char *value = dict->values[i];
    ret = reallocf(ret, strlen(ret) + strlen(keyb) + strlen(keya) + strlen(valb) + strlen(vala) + strlen(key) + strlen(value) + 1);
    strcat(ret, keyb);
    strcat(ret, key);
    strcat(ret, keya);
    strcat(ret, valb);
    strcat(ret, value);
    strcat(ret, vala);
  }
  ret = reallocf(ret, strlen(ret) + strlen(suffix) + 1);
  strcat(ret, suffix);
  return ret;
}

// }}}

kern_return_t get_unc_port(mach_port_t *port) {
  return bootstrap_look_up2(bootstrap_port, "com.apple.UNCUserNotification", port, 0, 0);
}

kern_return_t unc_send_dict(dict_t *dict, mach_msg_id_t flags, mach_msg_timeout_t timeout) {
  mach_port_t serverPort;
  mach_msg_base_t *msg;
  size_t size;
  char *data;
  kern_return_t kr;

  data = dict_to_xml(dict);

  if ((kr = get_unc_port(&serverPort)) != KERN_SUCCESS)
    goto bad;
  size = sizeof(mach_msg_base_t) + ((strlen(data) + 1 + 3) & (~0x3));
  msg = (mach_msg_base_t *)malloc(size);
  bzero(msg, size);
  msg->header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
  msg->header.msgh_size = size;
  msg->header.msgh_remote_port = serverPort;
  msg->header.msgh_id = flags;
  msg->body.msgh_descriptor_count = 0;
  strcpy((char *)((uint8_t *)msg + sizeof(mach_msg_base_t)), data);
  kr = mach_msg((mach_msg_header_t *)msg, MACH_SEND_MSG | (timeout == MACH_MSG_TIMEOUT_NONE ? 0 : MACH_SEND_TIMEOUT), size, 0, MACH_PORT_NULL, timeout, MACH_PORT_NULL);
  free(msg);
  mach_port_deallocate(mach_task_self(), serverPort);
bad:
  free(data);
  return kr;
}

kern_return_t UserNotice(mach_msg_timeout_t timeout, mach_msg_id_t flags, char *iconPath, char *soundPath, char *localizationPath, char *alertHeader, char *alertMessage, char *defaultButtonTitle) {
  dict_t *dict = new_dict();
  if (iconPath) dict_add_pair(dict, "IconURL", iconPath);
  if (soundPath) dict_add_pair(dict, "SoundURL", soundPath);
  if (localizationPath) dict_add_pair(dict, "LocalizationURL", localizationPath);
  if (alertHeader) dict_add_pair(dict, "AlertHeader", alertHeader);
  if (alertMessage) dict_add_pair(dict, "AlertMessage", alertMessage);
  if (defaultButtonTitle) dict_add_pair(dict, "DefaultButtonTitle", defaultButtonTitle);
  kern_return_t kr = unc_send_dict(dict, flags, timeout);
  dict_release(dict);
  return kr;
}

int main() {
  kern_return_t kr = UserNotice(MACH_MSG_TIMEOUT_NONE, 0, NULL, NULL, NULL, "meow", "cat", "ok");
  if (kr) {
    mach_error("UserNotice", kr);
  }
  return 0;
}
