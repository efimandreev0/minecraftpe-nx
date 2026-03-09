#ifndef _NP_MGR_H
#define _NP_MGR_H 1
extern "C" {

	typedef struct SceNpCommunicationId {
		char data[9];
		char term;
		unsigned char num;
		char dummy;
	} SceNpCommunicationId;

	typedef struct SceNpCommunicationPassphrase {
		unsigned char data[128];
	} SceNpCommunicationPassphrase;

	typedef struct SceNpCommunicationSignature {
		unsigned char data[160];
	} SceNpCommunicationSignature;

	typedef struct SceNpCommunicationConfig{
		const SceNpCommunicationId *comId;
		const SceNpCommunicationPassphrase *commPassphrase;
		const SceNpCommunicationSignature *commSignature;
	} SceNpCommunicationConfig;

	typedef struct SceNpOptParam {
		size_t optParamSize;
	} SceNpOptParam;

	typedef struct SceNpOnlineId {
		char data[16];
		char term;
		char dummy[3];
	} SceNpOnlineId;


	typedef struct SceNpId {
		SceNpOnlineId handle;
		unsigned char opt[8];
		unsigned char reserved[8];
	} SceNpId;

	typedef struct SceNpCountryCode {
		char data[2];
		char term;
		char padding[1];
	} SceNpCountryCode;

	int sceNpInit(const SceNpCommunicationConfig *commConf, SceNpOptParam *opt);
	void sceNpTerm();
	int sceNpManagerGetNpId(SceNpId *npId);
	int sceNpManagerGetAccountRegion(SceNpCountryCode *countryCode, int *languageCode);


}
#endif
