cat interfaces > /etc/network/interfaces
ifdown eth1 && ifup eth1
ifdown eth2 && ifup eth2
ifdown bond0 && ifup bond0
