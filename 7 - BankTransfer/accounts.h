/* accounts.h */
/* Header file to be used with
 * master.c and transfer.c
 */

struct ACCOUNTS {
	int nAccounts;		/* number of active accounts */
	int accounts[50];	/* space to hold up to 50 accounts */
};
