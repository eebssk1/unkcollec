#define UNICODE
#define __UNICODE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shlwapi.h>
#include <cJSON.h>

#define CallWithCheck(a,b) \
	if(a == NULL || a == 0) return errout(__FILE__,__LINE__,b)


int errout(const char* file, int line, LPCWSTR extra) {
	LPCWSTR e = (extra != NULL ? extra : L"None|无");
	int ssize = strlen(file)+7;
	char s[ssize] = {};
	sprintf(s,"%s:%d\n",file,line);
	TCHAR rs[ssize+wcslen(e)+1] = {};
	MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, s, strlen(s), rs, ssize);
	wcsncat(rs,e,wcslen(e));
	MessageBox(NULL,rs,NULL,MB_OK);
	return 0;
}

int notnonestring(const char* str) {
	if(str == NULL) return 0;
	int len = strlen(str);
	int es = 0;
	if(len<=0) return 0;
	for(int c = 0;c < len; c++) {
		if(str[c] == ' ') ++es;
	}
	return es == len ? 0 : 1;
}

__declspec(dllexport) int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
	int nowait = 0;
	int highprocess = 0;
	TCHAR exep[96] = {0};
	GetModuleFileName(NULL, exep, 96);
	PathRemoveFileSpec(exep);
	CallWithCheck(SetCurrentDirectory(exep),L"Failed to change dir|进入目录失败");
	FILE* fp = fopen("gstartup.json", "rb");
	CallWithCheck(fp,L"Failed to open startup file|打开启动描述配置失败 (gstartup.json)");
	fseek(fp, 0, SEEK_END);
	long fileSize = ftell(fp);
    	rewind(fp);
	char* buffer = (char*)malloc(fileSize + 1);
	if(!buffer) {fclose(fp);return errout(__FILE__,__LINE__,L"Memory Error|内存错误");}
	size_t readSize = fread(buffer, 1, fileSize, fp);
	fclose(fp);
	buffer[readSize] = '\0';
	cJSON *json = cJSON_Parse(buffer);
	free(buffer);
	CallWithCheck(json,L"Json Parse Error|启动描述配置解析失败 (gstartup.json)");
	const char* jstring = cJSON_Print(json);
	const cJSON *appname = NULL;
	const cJSON *cmdline = NULL;
	appname = cJSON_GetObjectItem(json, "appname");
	CallWithCheck(appname,L"Config not found|配置未找到 (appname)");
	cmdline = cJSON_GetObjectItem(json, "cmdline");
	CallWithCheck(cmdline,L"Config not found|配置未找到 (cmdline)");
	int cc = (cJSON_IsString(appname) && appname->valuestring != NULL) ? 1 : 0;
	if(cc != 0) cc = notnonestring(appname->valuestring);
	CallWithCheck(cc, L"Config Param Error|配置参数错误 (appname)");
	cc = (cJSON_IsString(cmdline) && cmdline->valuestring != NULL) ? 1 : 0;
        if(cc != 0) cc = notnonestring(cmdline->valuestring);
        CallWithCheck(cc, L"Config Param Error|配置参数错误 (cmdline)");
	const cJSON *env = NULL;
    	const cJSON *envn = NULL;
	env = cJSON_GetObjectItem(json, "env");
	if(env != NULL) {
		if(cJSON_IsArray(env))
		cJSON_ArrayForEach(envn, env) {
			int iserror = 0;
			cJSON *name = cJSON_GetObjectItem(envn, "name");
        		cJSON *value = cJSON_GetObjectItem(envn, "value");
			if(!cJSON_IsString(name) || name->valuestring == NULL) iserror = 1;
			if(!cJSON_IsString(value)) iserror = 1;
			if(!notnonestring(name->valuestring)) iserror = 1;
			if(iserror) {errout(__FILE__,__LINE__,L"Env define error|环境配置错误"); continue;}
			const char* namev = name->valuestring;
			const char* valuev = value->valuestring;
			int nt = strlen(namev) + 1;
			TCHAR namet[nt] = {};
			MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, namev, strlen(namev), namet, nt);
			LPCWSTR st0 = L"Env Define Failed|环境应用失败>";
			int st0l = wcslen(st0);
			if(valuev != NULL) {
				int vt = strlen(valuev) + 1;
                        	TCHAR valuet[vt] = {};
                        	MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, valuev, strlen(valuev), valuet, vt);
				LPCWSTR st1 = L"=";
				TCHAR err[nt + vt + st0l + wcslen(st1) + 1] = {};
				wcsncat(err,st0,st0l);
				wcsncat(err,namet,nt);
				wcsncat(err,st1,wcslen(st1));
				wcsncat(err,valuet,vt);
				if(!SetEnvironmentVariable(namet,valuet)) errout(__FILE__,__LINE__,err);
			} else {
				TCHAR err[nt + st0l + 1] = {};
				wcsncat(err,st0,st0l);
				wcsncat(err,namet,nt);
				if(!SetEnvironmentVariable(namet,NULL)) errout(__FILE__,__LINE__,err);
			}
		}
		else
		errout(__FILE__,__LINE__,L"Env define must be array|环境配置必须为列表");
	}
	const cJSON *nowaito = NULL;
	const cJSON *highpp = NULL;
	nowaito = cJSON_GetObjectItem(json, "nowait");
	highpp = cJSON_GetObjectItem(json, "highpp");
	if(nowaito != NULL) {
		if(cJSON_IsBool(nowaito)) {
			if(cJSON_IsTrue(nowaito)) nowait = 1;
		} else {
			errout(__FILE__,__LINE__,L"Config must be bool|配置必须为布尔 (nowait)");
		}
	}
	if(highpp != NULL) {
                if(cJSON_IsBool(highpp)) {
                        if(cJSON_IsTrue(highpp)) highprocess = 1;
                } else {
                        errout(__FILE__,__LINE__,L"Config must be bool|配置必须为布尔 (highpp)");
		}
        }
	const char* app = appname->valuestring;
	const char* cmd = cmdline->valuestring;
	int al = strlen(app)+1, cl = strlen(cmd)+1, jl = strlen(jstring)+1;
	TCHAR appt[al] = {};
	TCHAR cmdt[cl] = {};
	TCHAR jstt[jl] = {};
	MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, app, strlen(app), appt, al);
	MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, cmd, strlen(cmd), cmdt, cl);
	MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, jstring, strlen(jstring), jstt, jl);
	if(!PathIsRelative(appt)) return errout(__FILE__,__LINE__,L"Absolute app path not supported|绝对目标路径不被支持");
	int epl = wcslen(exep);
	TCHAR *quote = L"\"";
	TCHAR *space = L" ";
	TCHAR *bslas = L"\\";
	TCHAR rcmd[wcslen(quote)*2+wcslen(space)*2+wcslen(bslas)+epl+al+cl+wcslen(lpCmdLine)+1] = {};
	wcsncat(rcmd, quote, wcslen(quote));
	wcsncat(rcmd, exep, epl);
	wcsncat(rcmd, bslas, wcslen(bslas));
	wcsncat(rcmd, appt, al);
	wcsncat(rcmd, quote, wcslen(quote));
	wcsncat(rcmd, space, wcslen(space));
	wcsncat(rcmd, cmdt, cl);
	if(wcslen(lpCmdLine) > 0) {
		wcsncat(rcmd, space, wcslen(space));
		wcsncat(rcmd, lpCmdLine, wcslen(lpCmdLine));
	}
	const cJSON *debug = cJSON_GetObjectItem(json, "debug");
	FILE *f = NULL;
	if(debug != NULL) {
		if(cJSON_IsBool(nowaito)) {
			if(cJSON_IsTrue(debug)) {
				f = _wfopen(L"xjbqd_debug.log", L"w, ccs=UTF-8");
				if(!f) return errout(__FILE__,__LINE__,L"Failed to write debug file|写入调试日志失败");
				TCHAR *ss = L"%s\n";
				fwprintf(f,ss,L"Current Dir|当前目录");
				fwprintf(f,ss,exep);
				fwprintf(f,ss,L"Target Name|目标名称");
				fwprintf(f,ss,appt);
				fwprintf(f,ss,L"CmdLine|命令行");
				fwprintf(f,ss,cmdt);
				fwprintf(f,ss,L"Full CmdLine|完整命令行");
				fwprintf(f,ss,rcmd);
				fwprintf(f,ss,L"Json Config|勾儿配置");
				fwprintf(f,ss,jstt);
				fwprintf(f,ss,L"Current Env|当前环境");
				LPWSTR envStrings = GetEnvironmentStrings();
				const TCHAR* current = envStrings;
				TCHAR zr = '\0';
				while (*current != zr) {
					fwprintf(f,ss,current);
					current += wcslen(current) + 1;
				}
				FreeEnvironmentStrings(envStrings);
				fwprintf(f,L"\n");
				fflush(f);
			}
		} else {
			errout(__FILE__,__LINE__,L"Config must be bool|配置必须为布尔 (debug)");
		}
	}
	cJSON_Delete(json);
	SECURITY_ATTRIBUTES sa = {.nLength = sizeof(sa), .lpSecurityDescriptor = NULL, .bInheritHandle = TRUE};
	STARTUPINFOW si = {.dwFlags = STARTF_FORCEONFEEDBACK | STARTF_USEPOSITION, .dwX = 64, .dwY = 36};
	PROCESS_INFORMATION pi = {};
	si.cb = sizeof(si);
	int startup = 0;
	startup = CreateProcess(appt, rcmd, &sa, &sa, TRUE, (nowait != 1 ? CREATE_SUSPENDED : 0) | INHERIT_PARENT_AFFINITY | CREATE_PRESERVE_CODE_AUTHZ_LEVEL| CREATE_DEFAULT_ERROR_MODE | (highprocess == 1 ? ABOVE_NORMAL_PRIORITY_CLASS : 0), NULL, NULL, &si, &pi);
	if(startup <= 0) {
		errout(__FILE__,__LINE__,L"Run failed|启动失败");
		return errout(__FILE__,__LINE__,jstt);
	}
	if(f) fwprintf(f,L"PID|进程id> %d\n",pi.dwProcessId);
	if(nowait != 1) {
		if(f) fwprintf(f,L"Create job group|创建工作组\n");
		HANDLE job = CreateJobObject(&sa, NULL);
		if(!job && f) fwprintf(f,L"Create job group failed|创建工作组失败\n");
		if(job != NULL) {
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION jc = { };
			jc.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
			SetInformationJobObject(job, JobObjectExtendedLimitInformation, &jc, sizeof(jc));
			AssignProcessToJobObject(job, pi.hProcess);
			ResumeThread(pi.hThread);
		}
	}
	fflush(f);
	if(f) fwprintf(f,L"Start waiting|开始等待 nowait=%d\n",nowait);
	fflush(f);
	WaitForSingleObject(pi.hProcess, nowait == 1 ? 2500 : INFINITE);
	DWORD exi = -233;
	exi = GetExitCodeProcess(pi.hProcess, &exi) != 0 ? exi : -233;
	if(f) fwprintf(f,L"waiting finished with result|等待结束结果为> %d\n",exi);
	fclose(f);
	return exi;
}

