#include <psp2/io/stat.h>
#include <psp2/io/fcntl.h>

#include <psp2/net/http.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>

#include <psp2/sysmodule.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "graphics.h"

int file_copy(const char *src, const char *dst) {
	int ret;
	int read = 0;

	char buf[0x4000] = {0};

	int rfd = sceIoOpen(src, SCE_O_RDONLY, 6);
	if (rfd < 0) {
		return rfd;
	}

	int wfd = sceIoOpen(dst, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 6);
	if (wfd < 0) {
		sceIoClose(rfd);
		return wfd;
	}


	while (1) {

		int read = sceIoRead(rfd, &buf, sizeof(buf));

		if (read == 0 || read < 0)break;

		sceIoWrite(wfd, &buf, read);

	}

	sceIoClose(rfd);
	sceIoClose(wfd);

	return read;
}


int file_download(const char *src_url, const char *dst) {

	char net_init_buf[0x4000];

	SceNetInitParam netInitParam;
	netInitParam.memory = &net_init_buf;
	netInitParam.size = sizeof(net_init_buf);
	netInitParam.flags = 0;
	sceNetInit(&netInitParam);

	sceNetCtlInit();

	sceHttpInit(1*1024*1024);

	sceSslInit(300 * 1024);

	sceHttpsDisableOption(SCE_HTTPS_FLAG_SERVER_VERIFY);

	int tpl = sceHttpCreateTemplate("autoplugin_jp", 2, 1);
	if (tpl < 0) {
		return tpl;
	}
	int conn = sceHttpCreateConnectionWithURL(tpl, src_url, 0);
	if (conn < 0) {
		return conn;
	}
	int req = sceHttpCreateRequestWithURL(conn, 0, src_url, 0);
	if (req < 0) {
		return req;
	}
	int ret = sceHttpSendRequest(req, NULL, 0);
	if (ret < 0) {
		return ret;
	}

	char buf[0x4000] = {0};
	int read = 0;
	long long length = 0;

	ret = sceHttpGetResponseContentLength(req, &length);

	int fd = sceIoOpen(dst, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 6);

	if (fd < 0) {
		return fd;
	}

	while (1) {

		read = sceHttpReadData(req, buf, sizeof(buf));

		if (read == 0 || read < 0)
			break;

		sceIoWrite(fd, buf, read);

	}

	sceIoClose(fd);

	sceNetTerm();
	sceHttpTerm();

	return read;
}




int main(void) {

	int ret = 0;

	sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	sceSysmoduleLoadModule(SCE_SYSMODULE_HTTPS);

	ret = file_download("https://raw.githubusercontent.com/Umineko1993/Autoplugin-Lang-for-Japanese/master/JAPANESE.lua", "ux0:/app/AUTOPLUG0/lang/JAPANESE.lua");

	//ret = file_copy("app0:_", "ux0:/app/AUTOPLUG0/lang/JAPANESE.lua");

	if(ret >= 0){
		return 0;
	}

	psvDebugScreenInit();

	psvDebugScreenPrintf("Error code : 0x%X\n", ret);

	while (1) {}

	return 0;
}