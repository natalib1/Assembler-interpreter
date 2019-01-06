Valid1:		.data 4,53 , -3 , 23, -10000
Valid2:		.string "abcd"

			mov/0,0 	Valid1,r2
			.entry Valid1
			.extern externLab

			stop/0,0

			cmp/0,0 externLab,Valid2

Valid3:		rts/0,0

			cmp/0,0 Valid1{!Valid3},#-7
