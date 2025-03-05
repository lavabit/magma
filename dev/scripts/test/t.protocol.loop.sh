#!/bin/bash

# Name: t.pop.sh
# Author: Ladar Levison
#
# Description: Used for continuously testing the server via IMAP and POP.

echo ""

# Check and make sure magmad is running before attempting a connection.
PID=`pidof magmad magmad.check`       

# If the above logic doesn't find a process, then it's possible magma is running atop valgrind.
if [ -z "$PID" ]; then
	PID=`pidof valgrind`
	if [ ! -z "$PID" ]; then
		PID=`ps -ef | grep $PID | grep valgrind | grep -E "magmad|magmad.check" | awk -F' ' '{print $2}'`
	fi
fi

if [ -z "$PID" ]; then
	tput setaf 1; tput bold; echo "Magma process isn't running."; tput sgr0
	exit 2
fi

while true; do echo -ne '\ec' ; \
\
\
\
( sleep 3 ; printf "A01 LOGIN magma password\r\nA02 SELECT Inbox\r\nA03 STORE 1:10 FLAGS (\Deleted \Seen)\r\nA04 EXPUNGE\r\nA05 CLOSE\r\nA06 LOGOUT\r\n" | openssl s_client -ign_eof -starttls imap -connect localhost:9000 ; ) & disown ; printf "USER magma\r\nPASS password\r\nSTAT\r\nRETR 1\r\nRETR 2\r\nRETR 3\r\nRETR 4\r\nRETR 5\r\nRETR 6\r\nRETR 7\r\nRETR 8\r\nRETR 9\r\nRETR 10\r\nRETR 11\r\nRETR 12\r\nRETR 13\r\nRETR 14\r\nRETR 15\r\nRETR 16\r\nRETR 17\r\nRETR 18\r\nRETR 19\r\nRETR 20\r\nRETR 21\r\nRETR 22\r\nRETR 23\r\nRETR 24\r\nRETR 25\r\nRETR 26\r\nRETR 27\r\nRETR 28\r\nRETR 29\r\nRETR 30\r\nRETR 31\r\nRETR 32\r\nRETR 33\r\nRETR 34\r\nRETR 35\r\nRETR 36\r\nRETR 37\r\nRETR 38\r\nRETR 39\r\nRETR 40\r\nRETR 41\r\nRETR 42\r\nRETR 43\r\nRETR 44\r\nRETR 45\r\nRETR 46\r\nRETR 47\r\nRETR 48\r\nRETR 49\r\nRETR 50\r\nRETR 51\r\nRETR 52\r\nRETR 53\r\nRETR 54\r\nRETR 55\r\nRETR 56\r\nRETR 57\r\nRETR 58\r\nRETR 59\r\nRETR 60\r\nRETR 61\r\nRETR 62\r\nRETR 63\r\nRETR 64\r\nRETR 65\r\nRETR 66\r\nRETR 67\r\nRETR 68\r\nRETR 69\r\nRETR 70\r\nRETR 71\r\nRETR 72\r\nRETR 73\r\nRETR 74\r\nRETR 75\r\nRETR 76\r\nRETR 77\r\nRETR 78\r\nRETR 79\r\nRETR 80\r\nRETR 81\r\nRETR 82\r\nRETR 83\r\nRETR 84\r\nRETR 85\r\nRETR 86\r\nRETR 87\r\nRETR 88\r\nRETR 89\r\nRETR 90\r\nRETR 91\r\nRETR 92\r\nRETR 93\r\nRETR 94\r\nRETR 95\r\nRETR 96\r\nRETR 97\r\nRETR 98\r\nRETR 99\r\nRETR 100\r\nCAPA\r\nTOP 101 0\r\nTOP 102 0\r\nTOP 103 0\r\nTOP 104 0\r\nTOP 105 0\r\nTOP 106 0\r\nTOP 107 0\r\nTOP 108 0\r\nTOP 109 0\r\nTOP 110 0\r\nTOP 111 0\r\nTOP 112 0\r\nTOP 113 0\r\nTOP 114 0\r\nTOP 115 0\r\nTOP 116 0\r\nTOP 117 0\r\nTOP 118 0\r\nTOP 119 0\r\nTOP 120 0\r\nTOP 121 0\r\nTOP 122 0\r\nTOP 123 0\r\nTOP 124 0\r\nTOP 125 0\r\nTOP 126 0\r\nTOP 127 0\r\nTOP 128 0\r\nTOP 129 0\r\nTOP 130 0\r\nTOP 131 0\r\nTOP 132 0\r\nTOP 133 0\r\nTOP 134 0\r\nTOP 135 0\r\nTOP 136 0\r\nTOP 137 0\r\nTOP 138 0\r\nTOP 139 0\r\nTOP 140 0\r\nTOP 141 0\r\nTOP 142 0\r\nTOP 143 0\r\nTOP 144 0\r\nTOP 145 0\r\nTOP 146 0\r\nTOP 147 0\r\nTOP 148 0\r\nTOP 149 0\r\nTOP 150 0\r\nTOP 151 0\r\nTOP 152 0\r\nTOP 153 0\r\nTOP 154 0\r\nTOP 155 0\r\nTOP 156 0\r\nTOP 157 0\r\nTOP 158 0\r\nTOP 159 0\r\nTOP 160 0\r\nTOP 161 0\r\nTOP 162 0\r\nTOP 163 0\r\nTOP 164 0\r\nTOP 165 0\r\nTOP 166 0\r\nTOP 167 0\r\nTOP 168 0\r\nTOP 169 0\r\nTOP 170 0\r\nTOP 171 0\r\nTOP 172 0\r\nTOP 173 0\r\nTOP 174 0\r\nTOP 175 0\r\nTOP 176 0\r\nTOP 177 0\r\nTOP 178 0\r\nTOP 179 0\r\nTOP 180 0\r\nTOP 181 0\r\nTOP 182 0\r\nTOP 183 0\r\nTOP 184 0\r\nTOP 185 0\r\nTOP 186 0\r\nTOP 187 0\r\nTOP 188 0\r\nTOP 189 0\r\nTOP 190 0\r\nTOP 191 0\r\nTOP 192 0\r\nTOP 193 0\r\nTOP 194 0\r\nTOP 195 0\r\nTOP 196 0\r\nTOP 197 0\r\nTOP 198 0\r\nTOP 199 0\r\nTOP 200 0\r\nNOOP\r\nLIST 201\r\nLIST 202\r\nLIST 203\r\nLIST 204\r\nLIST 205\r\nLIST 206\r\nLIST 207\r\nLIST 208\r\nLIST 209\r\nLIST 210\r\nLIST 211\r\nLIST 212\r\nLIST 213\r\nLIST 214\r\nLIST 215\r\nLIST 216\r\nLIST 217\r\nLIST 218\r\nLIST 219\r\nLIST 220\r\nLIST 221\r\nLIST 222\r\nLIST 223\r\nLIST 224\r\nLIST 225\r\nLIST 226\r\nLIST 227\r\nLIST 228\r\nLIST 229\r\nLIST 230\r\nLIST 231\r\nLIST 232\r\nLIST 233\r\nLIST 234\r\nLIST 235\r\nLIST 236\r\nLIST 237\r\nLIST 238\r\nLIST 239\r\nLIST 240\r\nLIST 241\r\nLIST 242\r\nLIST 243\r\nLIST 244\r\nLIST 245\r\nLIST 246\r\nLIST 247\r\nLIST 248\r\nLIST 249\r\nLIST 250\r\nLIST 251\r\nLIST 252\r\nLIST 253\r\nLIST 254\r\nLIST 255\r\nLIST 256\r\nLIST 257\r\nLIST 258\r\nLIST 259\r\nLIST 260\r\nLIST 261\r\nLIST 262\r\nLIST 263\r\nLIST 264\r\nLIST 265\r\nLIST 266\r\nLIST 267\r\nLIST 268\r\nLIST 269\r\nLIST 270\r\nLIST 271\r\nLIST 272\r\nLIST 273\r\nLIST 274\r\nLIST 275\r\nLIST 276\r\nLIST 277\r\nLIST 278\r\nLIST 279\r\nLIST 280\r\nLIST 281\r\nLIST 282\r\nLIST 283\r\nLIST 284\r\nLIST 285\r\nLIST 286\r\nLIST 287\r\nLIST 288\r\nLIST 289\r\nLIST 290\r\nLIST 291\r\nLIST 292\r\nLIST 293\r\nLIST 294\r\nLIST 295\r\nLIST 296\r\nLIST 297\r\nLIST 298\r\nLIST 299\r\nLIST 300\r\nUIDL 301\r\nUIDL 302\r\nUIDL 303\r\nUIDL 304\r\nUIDL 305\r\nUIDL 306\r\nUIDL 307\r\nUIDL 308\r\nUIDL 309\r\nUIDL 310\r\nUIDL 311\r\nUIDL 312\r\nUIDL 313\r\nUIDL 314\r\nUIDL 315\r\nUIDL 316\r\nUIDL 317\r\nUIDL 318\r\nUIDL 319\r\nUIDL 320\r\nUIDL 321\r\nUIDL 322\r\nUIDL 323\r\nUIDL 324\r\nUIDL 325\r\nUIDL 326\r\nUIDL 327\r\nUIDL 328\r\nUIDL 329\r\nUIDL 330\r\nUIDL 331\r\nUIDL 332\r\nUIDL 333\r\nUIDL 334\r\nUIDL 335\r\nUIDL 336\r\nUIDL 337\r\nUIDL 338\r\nUIDL 339\r\nUIDL 340\r\nUIDL 341\r\nUIDL 342\r\nUIDL 343\r\nUIDL 344\r\nUIDL 345\r\nUIDL 346\r\nUIDL 347\r\nUIDL 348\r\nUIDL 349\r\nUIDL 350\r\nUIDL 351\r\nUIDL 352\r\nUIDL 353\r\nUIDL 354\r\nUIDL 355\r\nUIDL 356\r\nUIDL 357\r\nUIDL 358\r\nUIDL 359\r\nUIDL 360\r\nUIDL 361\r\nUIDL 362\r\nUIDL 363\r\nUIDL 364\r\nUIDL 365\r\nUIDL 366\r\nUIDL 367\r\nUIDL 368\r\nUIDL 369\r\nUIDL 370\r\nUIDL 371\r\nUIDL 372\r\nUIDL 373\r\nUIDL 374\r\nUIDL 375\r\nUIDL 376\r\nUIDL 377\r\nUIDL 378\r\nUIDL 379\r\nUIDL 380\r\nUIDL 381\r\nUIDL 382\r\nUIDL 383\r\nUIDL 384\r\nUIDL 385\r\nUIDL 386\r\nUIDL 387\r\nUIDL 388\r\nUIDL 389\r\nUIDL 390\r\nUIDL 391\r\nUIDL 392\r\nUIDL 393\r\nUIDL 394\r\nUIDL 395\r\nUIDL 396\r\nUIDL 397\r\nUIDL 398\r\nUIDL 399\r\nUIDL 400\r\nNOOP\r\nDELE 1\r\nDELE 2\r\nDELE 3\r\nDELE 4\r\nDELE 5\r\nDELE 6\r\nDELE 7\r\nDELE 8\r\nDELE 9\r\nDELE 10\r\nDELE 11\r\nDELE 12\r\nDELE 13\r\nDELE 14\r\nDELE 15\r\nDELE 16\r\nDELE 17\r\nDELE 18\r\nDELE 19\r\nDELE 20\r\nDELE 21\r\nDELE 22\r\nDELE 23\r\nDELE 24\r\nDELE 25\r\nDELE 26\r\nDELE 27\r\nDELE 28\r\nDELE 29\r\nDELE 30\r\nDELE 31\r\nDELE 32\r\nDELE 33\r\nDELE 34\r\nDELE 35\r\nDELE 36\r\nDELE 37\r\nDELE 38\r\nDELE 39\r\nDELE 40\r\nDELE 41\r\nDELE 42\r\nDELE 43\r\nDELE 44\r\nDELE 45\r\nDELE 46\r\nDELE 47\r\nDELE 48\r\nDELE 49\r\nDELE 50\r\nDELE 51\r\nDELE 52\r\nDELE 53\r\nDELE 54\r\nDELE 55\r\nDELE 56\r\nDELE 57\r\nDELE 58\r\nDELE 59\r\nDELE 60\r\nDELE 61\r\nDELE 62\r\nDELE 63\r\nDELE 64\r\nDELE 65\r\nDELE 66\r\nDELE 67\r\nDELE 68\r\nDELE 69\r\nDELE 70\r\nDELE 71\r\nDELE 72\r\nDELE 73\r\nDELE 74\r\nDELE 75\r\nDELE 76\r\nDELE 77\r\nDELE 78\r\nDELE 79\r\nDELE 80\r\nDELE 81\r\nDELE 82\r\nDELE 83\r\nDELE 84\r\nDELE 85\r\nDELE 86\r\nDELE 87\r\nDELE 88\r\nDELE 89\r\nDELE 90\r\nDELE 91\r\nDELE 92\r\nDELE 93\r\nDELE 94\r\nDELE 95\r\nDELE 96\r\nDELE 97\r\nDELE 98\r\nDELE 99\r\nDELE 100\r\nQUIT\r\n" | openssl s_client -quiet -connect localhost:8500 -quiet 2>&1 ; \
\
\
\
printf "A01 LOGIN magma password\r\nA02 LIST \"\" \"*\"\r\nA03 SELECT Inbox\r\nA04 FETCH 1:100 RFC822\r\nA05 STORE 1:50 FLAGS (\Deleted \Flagged)\r\nA06 STORE 51:100 FLAGS (\Deleted \Answered)\r\nA07 EXPUNGE\r\nA08 ID\r\nA09 CAPABILITY\r\nA10 NOOP\r\nA11 STATUS\r\nA12 FETCH 1:100 (UID BODY[HEADER.FIELDS (SUBJECT DATE)])\r\nA13 STORE 1:100 FLAGS.SILENT (\Deleted)\r\nA14 CLOSE\r\nA15 LOGOUT\r\n" | openssl s_client -ign_eof -starttls imap -connect localhost:9000 ;
\
\
\
done



