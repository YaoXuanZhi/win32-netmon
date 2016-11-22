// Copyright (C) 2012-2014 F32 (feng32tc@gmail.com)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 

#include "stdafx.h"
#include "PortCache.h"

PortCache::PortCache()
{
    RtlZeroMemory(_tcpPortTable, sizeof(_tcpPortTable));
    RtlZeroMemory(_udpPortTable, sizeof(_udpPortTable));
}

int PortCache::GetTcpPortPid(int port)
{
    if( _tcpPortTable[port] != 0 )
    {
        return _tcpPortTable[port];
    }
    else
    {
        // Rebuild Cache
        RebuildTcpTable();
        
        // Return
        return _tcpPortTable[port];
    }
}

int PortCache::GetUdpPortPid(int port)
{
    if( _udpPortTable[port] != 0 )
    {
        return _udpPortTable[port];
    }
    else
    {
        // Rebuild Cache
        RebuildUdpTable();

        // Return
        return _udpPortTable[port];
    }
}

//ԭ���ģ�����ANY_SIZE�ڵ�ǰ��Ŀ��Ĭ��Ϊ1�����Լ���ע�͵���
//#define  ANY_SIZE 1024����ô�˳����޷�������ȡ�����
//�е����������Ϣ������MSDN��GetTcpTable���÷���д�˴˺�����
//������ʾ��
void PortCache::RebuildTcpTable()
{
	// Clear the table
	RtlZeroMemory(_tcpPortTable, sizeof(_tcpPortTable));

	DWORD dwRetValue = NO_ERROR;
	DWORD dwBufferSize = 0;

	PMIB_TCPTABLE_OWNER_PID pTable = NULL;
	//��һ�λ�ȡ�����Ի��pTable������ڴ��С����dwBufferSize�������
	dwRetValue = GetExtendedTcpTable(NULL, &dwBufferSize,
		FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
	if (dwRetValue != ERROR_INSUFFICIENT_BUFFER)
	{
		printf("Failed to snapshot TCP endpoints.\n");
		return;
	}

	//ʹ��HeapAlloc�ڶ��Ϸ���һ��dwBufferSize��С���ڴ�ռ��pTable����
	pTable = (PMIB_TCPTABLE_OWNER_PID)HeapAlloc(GetProcessHeap(), 0, dwBufferSize);
	//�ڶ��λ�ȡ����TCP��ص�������̿��ձ��浽pTable������
	dwRetValue = GetExtendedTcpTable(pTable, &dwBufferSize,
		FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);

	if (dwRetValue != NO_ERROR)
	{
		printf("Failed to snapshot TCP endpoints.\n");
		HeapFree(GetProcessHeap(), 0, pTable);
		return;
	}

	// Rebuild the table
	for (unsigned int i = 0; i < pTable->dwNumEntries; i++)
	{
		_tcpPortTable[ntohs((unsigned short)pTable->table[i].dwLocalPort)] =
			pTable->table[i].dwOwningPid;
	}
	HeapFree(GetProcessHeap(), 0, pTable);
}

void PortCache::RebuildUdpTable()
{
	// Clear the table
	RtlZeroMemory(_udpPortTable, sizeof(_udpPortTable));

	DWORD dwRetValue = NO_ERROR;
	DWORD dwBufferSize = 0;

	//������ͬRebuildTcpTable
	PMIB_UDPTABLE_OWNER_PID pTable = NULL;
	dwRetValue = GetExtendedUdpTable(NULL, &dwBufferSize,
		FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);
	if (dwRetValue != ERROR_INSUFFICIENT_BUFFER)
	{
		printf("Failed to snapshot UDP endpoints.\n");
		return;
	}

	//������ͬRebuildTcpTable
	pTable = (PMIB_UDPTABLE_OWNER_PID)HeapAlloc(GetProcessHeap(), 0, dwBufferSize);
	dwRetValue = GetExtendedUdpTable(pTable, &dwBufferSize,
		FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);

	if (dwRetValue != NO_ERROR)
	{
		printf("Failed to snapshot UDP endpoints.\n");
		HeapFree(GetProcessHeap(), 0, pTable);
		return;
	}

	// Rebuild the table
	for (unsigned int i = 0; i < pTable->dwNumEntries; i++)
	{
		_udpPortTable[ntohs((unsigned short)pTable->table[i].dwLocalPort)] =
			pTable->table[i].dwOwningPid;
	}
	HeapFree(GetProcessHeap(), 0, pTable);
}