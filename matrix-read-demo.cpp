// Performance demo for various methods of reading integer data in text form from disk 
// https://github.com/av-pavlov/matrix-read-demo

#include <cstdio>
#include <iostream>
#include <fstream>
#include <ctime>
#include <sys/stat.h>

#define		is_num(c)		((c) >= '0' && (c) <= '9')

using namespace std;

bool save_file_fstream(unsigned N) {
	unsigned i, j;
	ofstream f("data.txt");
	if (f.bad()) return false;
	for (i = 0; i < N; ++i) {
		for (j = 0; j < N; ++j) 
			f << (i*j) % 1000U << " ";
		f << "\n";
	}
	f.close();
	return true;
}

bool save_file_fprintf(unsigned N) {
	unsigned i, j;
	FILE *f;
	if (fopen_s(&f, "data.txt", "w")) {
		cout << "error opening data.txt!\n";
		return false;
	}
	for (i = 0; i < N; ++i) {
		for (j = 0; j < N; ++j)
			fprintf(f, "%d ", (i*j) % 1000U);
		fprintf(f, "\n");
	}
	fclose(f);
	return true;
}

void read_file_fstream(int **A, unsigned N) {
	unsigned i, j;
	ifstream f("data.txt");
	for (i = 0; i < N; ++i)
		for (j = 0; j < N; ++j)
			f >> A[i][j];
	f.close();
}

void read_file_fscanf(int **A, unsigned N) {
	unsigned i, j;
	FILE* f; if (fopen_s(&f, "data.txt", "r")) {
		cout << "error opening data.txt!\n";
		return;
	}
	
	for (i = 0; i < N; ++i)
		for (j = 0; j < N; ++j)
			fscanf_s(f, "%d", &(A[i][j]));
	fclose(f);
}

void read_file_in_memory(int **A, unsigned N) {
	struct _stat info;
	_stat("data.txt", &info);
	off_t size = info.st_size;
	char* buf = new char[size];
	FILE* f;
	if (fopen_s(&f, "data.txt", "rb")) {
		cout << "error opening data.txt!\n";
		return;
	}
	off_t nread = fread(buf, 1, size, f);
	fclose(f);
	char *p = buf, *buf_end = buf + nread;

	unsigned i = 0, j = 0, n = N;
	while (!is_num(*p)) ++p;
	while (p < buf_end && i < n) {
		A[i][j] = *p++ - '0';
		while (is_num(*p))
			A[i][j] = 10 * A[i][j] + (*p++ - '0');

		j = (j + 1) % n;
		if (j == 0) ++i;
		while (p < buf_end && !is_num(*p)) ++p;
	}
	delete[] buf;
}

void read_file_in_asm(int** A, unsigned N) {
	struct _stat info;
	_stat("data.txt", &info);
	off_t size = info.st_size;
	char* buf = new char[size];
	FILE* f;
	if (fopen_s(&f, "data.txt", "rb")) {
		cout << "error opening data.txt!\n";
		return;
	}
	off_t nread = fread(buf, 1, size, f);
	fclose(f);
	
	char *buf_end = buf + nread;
	unsigned i = 0, n = N;

	__asm {
		mov		edx, dword ptr[buf_end];// edx = buf_end

		mov		eax, dword ptr[buf];	// eax = buf[0] - 1
		dec		eax;
		mov		esi, dword ptr[A];		// esi = *(A[0][0])
		mov		esi, dword ptr[esi];	 
	
	setup_rowend:
		mov		edi, dword ptr[n];		// edi = end of row pointed by esi
		lea		edi, [4*edi + esi];		 
	
	skip_nondigits:
		inc		eax;
		cmp		eax, edx;				// p >= buf_end?
		je		data_eof;
		
		movsx	ebx, byte ptr[eax];		// ebx = *p
		sub		ebx, 30h;				// ebx = *p - '0'
		cmp		ebx, 0;					// ebx >= 0?
		jl		next_nondigit;				// if no...
		cmp		ebx, 9;					// ebx <= 9?
		jle		first_digit;			// if yes

	next_nondigit:
		add		eax, 1;					// p++;
		jmp		skip_nondigits;

	first_digit:
		mov		ecx, ebx;				// ecx = A[i][j] = *p-'0';

	process_digits:
		add		eax, 1;					// p++;
		cmp		eax, edx;				// if p>=buf_end...
		je		data_eof;

		movsx	ebx, byte ptr[eax];		// ebx = *p
		sub		ebx, 30h;				// ebx = *p - '0'
		cmp		ebx, 0;					// ebx >= 0?
		jl		first_nondigit;			//	 if no, it's a nondigit
		cmp		ebx, 9;					// ebx <= 9?
		jg		first_nondigit;			//	 if no, it's a nondigit

		imul	ecx, 10;				// ecx = 10*ecx + ebx
		add		ecx, ebx;
		jmp		process_digits;

	first_nondigit:
		mov		dword ptr [esi], ecx;   // A[i][j] = ecx
		add		esi, 4;
		
		cmp		esi, edi;				// j>=n?
		jl		skip_nondigits;			// if not, go on

	//  ++i
		mov		ecx, dword ptr [i];		// ++i
		inc		ecx;
		mov		dword ptr[i], ecx;

	// if (i<n) { esi = &A[i][0]; edi = esi + n;}
		cmp		ecx, dword ptr [n];		// if i==n, exit
		je		data_eof;

		mov		esi, dword ptr[A];		// eax = A[0]
		mov		esi, [4*ecx + esi]		// ecx = i*4
		
		jmp setup_rowend;

	data_eof:
	}
	delete[] buf;
	fclose(f);
}

void test_print_data(int **A, unsigned N) {
	unsigned i, j;
	cout << "\nenter i, j: ";
	cin >> i >> j;
	cout << " A[" << i << "][" << j << "] = " << A[i][j] << "\n";
}

void renewN(int **&A, unsigned &N, unsigned &old_N)
{
	cout << "Choose new matrix size: ";
	cin >> N;
	if (A != nullptr) {
		for (unsigned i = 0; i < old_N; ++i)
			delete[] A[i];
		delete[] A;
	}
	A = new int*[N];
	for (unsigned i = 0; i < N; ++i)
		A[i] = new int[N];
	old_N = N;
}

int main()
{
	unsigned choice, N = 0, old_N = 0, saved_N = 0;
	int **A = nullptr;

	do {
		cout << "\nDemo Lab 3 Reading an NxN integer matrix from a text file\n" <<
			      "=========================================================\n";
		if (N == 0) choice = 1; else {
			cout <<
				"MENU (N="<< N << "):\n "
				"1) Choose new N\n " <<
				"2) Generate file data.txt in C fprintf\n " <<
				"3) Generate file data.txt in C++ fstream\n ";
			if (saved_N == N) cout <<
				"4) Read file data.txt in C fscanf\n "
				"5) Read file data.txt in C++ fstream\n " <<
				"6) Read and parse file data.txt in memory in C++\n " <<
				"7) Read file data.txt in memory and parse in ASM\n " <<
				"8) Test: print a matrix element\n ";
			cout <<
				"0) Exit\n\n" <<
				"Enter choice:";
			cin >> choice;
		};
		
		clock_t t0 = clock();
		
		switch (choice) {
			case 1: renewN(A, N, old_N);    	break;
			case 2: if (save_file_fprintf(N)) saved_N = N; break;
			case 3: if (save_file_fstream(N)) saved_N = N; break;
			case 4: read_file_fscanf(A, N);		break;
			case 5: read_file_fstream(A, N);	break;
			case 6: read_file_in_memory(A, N);	break;
			case 7: read_file_in_asm(A, N);		break;
			case 8: test_print_data(A, N);		break;
		}

		if (choice > 1 && choice < 8)
			cout << "Done, " << clock() - t0 << "ms\n";
	} while (choice != 0);

	return 0;
}
