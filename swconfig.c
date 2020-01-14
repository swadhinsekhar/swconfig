#include <stdlib.h>
#include <string.h>

#include <rtk_error.h>
#include <rtk_types.h>
#include <rtk_switch.h>
#include <port.h>
#include <smi.h>
#include <stat.h>
#include <vlan.h>
#include <svlan.h>
#include <rtl8367c_asicdrv_mib.h>
#include <l2.h>

#define EX_D(expr)  \
{   \
    if (!(expr)) {  \
        printf("[ OK ] ["#expr"]\n");   \
    } else {    \
        printf("[Fail] ["#expr"]\n");   \
    }   \
}

#define PX_MAC		"%02x:%02x:%02x:%02x:%02x:%02x"
#define PS_MAC(mac) (unsigned char)((unsigned char *)mac)[0],(unsigned char)((unsigned char *)mac)[1],\
                    (unsigned char)((unsigned char *)mac)[2],(unsigned char)((unsigned char *)mac)[3],\
                    (unsigned char)((unsigned char *)mac)[4], (unsigned char)((unsigned char *)mac)[5]

#define PX_IP		"%d.%d.%d.%d"
#define PS_IP(ip)	(unsigned char)((unsigned char *)&(ip))[0],(unsigned char)((unsigned char *)&(ip))[1],\
                    (unsigned char)((unsigned char *)&(ip))[2],(unsigned char)((unsigned char *)&(ip))[3]

void bsp_vlans_l2forward()
{
	rtk_vlan_init();
}

void bsp_vlans_isolate()
{
	int             i = 0;
	rtk_vlan_cfg_t  vlan_cfg;
	rtk_vlan_t      vlan_id;
	rtk_port_t      port;

	rtk_vlan_init();

	rtk_port_t ports_out[2] = { UTP_PORT0, UTP_PORT2};

	for (i = 0; i < 2; i++){
		port = ports_out[i];
		vlan_id = 100 + port;
		memset(&vlan_cfg, 0x00, sizeof(rtk_vlan_cfg_t));
		RTK_PORTMASK_PORT_SET(vlan_cfg.mbr, port);
		RTK_PORTMASK_PORT_SET(vlan_cfg.mbr, UTP_PORT1);
		RTK_PORTMASK_PORT_SET(vlan_cfg.untag, port);
		vlan_cfg.ivl_en = 1;
		rtk_vlan_set(vlan_id, &vlan_cfg);
		rtk_vlan_portPvid_set(port, vlan_id, 0);
	}
}

void bsp_vlan_qinq_config_test1()
{
	EX_D(rtk_svlan_init());
	EX_D(rtk_svlan_tpidEntry_set(0x88a8));

	EX_D(rtk_svlan_servicePort_add(UTP_PORT1));
	//EX_D(rtk_svlan_servicePort_add(UTP_PORT2));

	rtk_svlan_memberCfg_t svlanCfg;

	memset(&svlanCfg, 0x00, sizeof(rtk_svlan_memberCfg_t));
	svlanCfg.svid = 10;
	svlanCfg.fid = 0;
	svlanCfg.priority = 0;
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, UTP_PORT0);
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, UTP_PORT1);
	RTK_PORTMASK_PORT_SET(svlanCfg.untagport, UTP_PORT0);
	EX_D(rtk_svlan_memberPortEntry_set(10, &svlanCfg));
	EX_D(rtk_svlan_defaultSvlan_set(UTP_PORT0, 10));

	memset(&svlanCfg, 0x00, sizeof(rtk_svlan_memberCfg_t));
	svlanCfg.svid = 12;
	svlanCfg.fid = 0;
	svlanCfg.priority = 0;
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, UTP_PORT2);
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, UTP_PORT1);
	RTK_PORTMASK_PORT_SET(svlanCfg.untagport, UTP_PORT2);
	EX_D(rtk_svlan_memberPortEntry_set(12, &svlanCfg));
	EX_D(rtk_svlan_defaultSvlan_set(UTP_PORT2, 12));
}

void bsp_vlan_qinq_config()
{
	EX_D(rtk_svlan_init());
	EX_D(rtk_svlan_tpidEntry_set(0x8100));

	EX_D(rtk_svlan_servicePort_add(EXT_PORT0));

	rtk_svlan_memberCfg_t svlanCfg;

	memset(&svlanCfg, 0x00, sizeof(rtk_svlan_memberCfg_t));
	svlanCfg.svid = 100;
	svlanCfg.fid = 0;
	svlanCfg.priority = 0;
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, UTP_PORT0);
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, EXT_PORT0);
	RTK_PORTMASK_PORT_SET(svlanCfg.untagport, UTP_PORT0);
	EX_D(rtk_svlan_memberPortEntry_set(100, &svlanCfg));
	EX_D(rtk_svlan_defaultSvlan_set(UTP_PORT0, 100));

	memset(&svlanCfg, 0x00, sizeof(rtk_svlan_memberCfg_t));
	svlanCfg.svid = 101;
	svlanCfg.fid = 0;
	svlanCfg.priority = 0;
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, UTP_PORT1);
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, EXT_PORT0);
	RTK_PORTMASK_PORT_SET(svlanCfg.untagport, UTP_PORT1);
	EX_D(rtk_svlan_memberPortEntry_set(101, &svlanCfg));
	EX_D(rtk_svlan_defaultSvlan_set(UTP_PORT1, 101));

	memset(&svlanCfg, 0x00, sizeof(rtk_svlan_memberCfg_t));
	svlanCfg.svid = 102;
	svlanCfg.fid = 0;
	svlanCfg.priority = 0;
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, UTP_PORT2);
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, EXT_PORT0);
	RTK_PORTMASK_PORT_SET(svlanCfg.untagport, UTP_PORT2);
	EX_D(rtk_svlan_memberPortEntry_set(102, &svlanCfg));
	EX_D(rtk_svlan_defaultSvlan_set(UTP_PORT2, 102));

	memset(&svlanCfg, 0x00, sizeof(rtk_svlan_memberCfg_t));
	svlanCfg.svid = 103;
	svlanCfg.fid = 0;
	svlanCfg.priority = 0;
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, UTP_PORT3);
	RTK_PORTMASK_PORT_SET(svlanCfg.memberport, EXT_PORT0);
	RTK_PORTMASK_PORT_SET(svlanCfg.untagport, UTP_PORT3);
	EX_D(rtk_svlan_memberPortEntry_set(103, &svlanCfg));
	EX_D(rtk_svlan_defaultSvlan_set(UTP_PORT3, 103));
}


void  bsp_l2table_show()
{
	rtk_uint32          address = 0;
	rtk_l2_ucastAddr_t  l2_data;
	rtk_api_ret_t       ret;

	while (1) {
	    if ((ret = rtk_l2_addr_next_get(READMETHOD_NEXT_L2UC,
                    UTP_PORT0, &address, &l2_data))) {
			break;
		}
		printf(PX_MAC" %d %d %d \n",
            PS_MAC(&(l2_data.mac)), l2_data.ivl,
            l2_data.cvid, l2_data.port);
		address++;
	}
}

void bsp_l2table_clear()
{
	rtk_uint32 address = 0;
	rtk_l2_ucastAddr_t l2_data;
	rtk_api_ret_t ret;

	while (1){
		if ((ret = rtk_l2_addr_next_get(READMETHOD_NEXT_L2UC, UTP_PORT0, &address, &l2_data))){
			break;
		}
		rtk_l2_addr_del(&(l2_data.mac), &l2_data);
		printf(PX_MAC" %d %d %d\n",
            PS_MAC(&(l2_data.mac)),
            l2_data.ivl, l2_data.cvid, l2_data.port);
		address++;
	}
}

void bsp_ports_status()
{
	int                     i = 0;
	rtk_port_linkStatus_t   linkStatus;
	rtk_port_speed_t        speed;
	rtk_port_duplex_t       duplex;
	char                    *linkStatus_str[] = { "DOWN", "  UP" };
	char                    *speed_str[] = { "  10M", " 100M", "1000M" };
	char                    *duplex_str[] = { "HALF_DUPLEX ", "FULL_DUPLEX" };

	rtk_port_t ports[5] = { UTP_PORT0, UTP_PORT1, UTP_PORT2, UTP_PORT3, EXT_PORT0};

	for (i = 0; i < 4; i++) {
		rtk_port_phyStatus_get(ports[i], &linkStatus, &speed, &duplex);
		printf("[port%d] %s   %s   %s \n",
            ports[i], linkStatus_str[linkStatus],
            speed_str[speed], duplex_str[duplex]);
	}
}

static const char *rtk_port_str[4] = {
    "port0",
    "port1",
    "port2",
    "port3"
};

rtk_port_t get_swport(char *swport)
{
    int i;

    for(i=0; i<4; i++) {
        if(!strcmp(rtk_port_str[i], swport)) {
            return i;
        }
    }

    return RTK_PORT_MAX;
}

void bsp_switch_ports_status(char *swport)
{
	rtk_port_linkStatus_t   linkStatus;
	rtk_port_speed_t        speed;
	rtk_port_duplex_t       duplex;
	char                    *linkStatus_str[] = { "DOWN", "  UP" };
	char                    *speed_str[] = { "  10M", " 100M", "1000M" };
	char                    *duplex_str[] = { "HALF_DUPLEX ", "FULL_DUPLEX" };

	rtk_port_t ports = get_swport(swport);
    if(RTK_PORT_MAX != ports) {
		rtk_port_phyStatus_get(ports, &linkStatus, &speed, &duplex);
		printf("[%s] %s   %s   %s \n",
            swport, linkStatus_str[linkStatus],
            speed_str[speed], duplex_str[duplex]);
    } else {
        printf("Invalid switch ports : %s\n", swport);
    }
}

void bsp_ports_info()
{
	int         i = 0;
	rtk_port_t  ports[5] = { UTP_PORT0, UTP_PORT1, UTP_PORT2, UTP_PORT3, EXT_PORT0};

	printf("------------------------------------------------------------- \n");
	printf("port   TX/RX     Octets      UcastPkts   MulticastPkts   BroadcastPkts \n");
	for (i = 0; i < 4; i++) {
		rtk_stat_counter_t InOctets;;
		rtk_stat_counter_t InUcastPkts;
		rtk_stat_counter_t InMulticastPkts;
		rtk_stat_counter_t InBroadcastPkts;
		rtk_stat_counter_t OutOctets;
		rtk_stat_counter_t OutUcastPkts;
		rtk_stat_counter_t OutMulticastPkts;
		rtk_stat_counter_t OutBroadcastPkts;

		rtk_stat_port_get(ports[i], ifInOctets, &InOctets);
		rtk_stat_port_get(ports[i], ifInUcastPkts, &InUcastPkts);
		rtk_stat_port_get(ports[i], ifInMulticastPkts, &InMulticastPkts);
		rtk_stat_port_get(ports[i], ifInBroadcastPkts, &InBroadcastPkts);
	    printf("[port%d] Rx %12llu %12llu %12llu %12llu \n", ports[i],
			InOctets, InUcastPkts, InMulticastPkts, InBroadcastPkts);

		rtk_stat_port_get(ports[i], ifOutOctets, &OutOctets);
		rtk_stat_port_get(ports[i], ifOutUcastPkts, &OutUcastPkts);
		rtk_stat_port_get(ports[i], ifOutMulticastPkts, &OutMulticastPkts);
		rtk_stat_port_get(ports[i], ifOutBroadcastPkts, &OutBroadcastPkts);
		printf("[Port%d] Tx %12llu %12llu %12llu %12llu \n", ports[i],
			OutOctets, OutUcastPkts, OutMulticastPkts, OutBroadcastPkts);
	}
}

void bsp_info()
{
	unsigned int addr = 0x1b03;
	unsigned int data = 0;

	smi_read(addr, &data);
	printf("Reg[%x] Val: %x \n", addr, data);
}

void help()
{
	//printf("isolate");
	//printf("l2forward");
	printf("isolate2\n");
	printf("port-status\n");
	printf("port-status portX\n");
	printf("arp-show\n");
	printf("info\n");
}

int main(int argc, char **argv)
{
	rtk_switch_init();
	rtk_l2_init();

	if (argc == 2) {
		char * cmd = argv[1];

		if (strcmp(cmd, "help") == 0) {
			help();
		} else if (strcmp(cmd, "isolate") == 0) {
			//bsp_vlans_isolate();
            ;
		} else if (strcmp(cmd, "isolate2") == 0) {
			bsp_vlan_qinq_config();
		} else if (strcmp(cmd, "l2forward") == 0) {
			//bsp_vlans_l2forward();
            ;
		} else if (strcmp(cmd, "port-status") == 0) {
            bsp_ports_status();
		} else if (strcmp(cmd, "arp-show") == 0) {
			bsp_l2table_show();
		} else if (strcmp(cmd, "info") == 0) {
			bsp_info();
			bsp_ports_info();
		} else {
			help();
		}

		return 0;
	}

    if (argc == 3) {
		char *cmd1 = argv[1];
		char *cmd2 = argv[2];

        if((NULL != cmd1) && (NULL != cmd2)) {
		    if (!strcmp(cmd1, "port-status")) {
                bsp_switch_ports_status(cmd2);
                return 0;
            }
        }
    }

	help();

	return 0;
}

