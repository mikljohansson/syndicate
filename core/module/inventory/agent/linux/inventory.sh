#!/bin/bash
# Inventory agent script
#
# The inventory agent collects host information using dmidecode, 
# ddcprobe, lspci, ifconfig, uname, rpm, cat /proc, ... and 
# transmits it to a central inventory server using curl.
#
# dmidecode and ddcprobe needs to be run as root or have some 
# sudo configuration setup.
#
# @access		public
# @package		synd.core.module

while getopts si:d name; do
	case "$name" in
		s)
			skipsoftware=1
			;;
		i)
			if [ -z "$OPTARG" ] || [ ! -r "$OPTARG" ]; then
				echo >&2 "Error: File '$OPTARG' does not exist or is not readable"
				exit 1
			fi
			loadedimage="$OPTARG"
			;;
		d)
			debug=1
			;;
	esac
done
shift $(($OPTIND - 1))

if [ -z "$1" ]; then
	echo >&2 "Usage: inventory.sh [OPTION]... URI"
	echo >&2 "Collects hardware and software information and uploads it to server"
	echo >&2 "Example: inventory.sh http://www.example.com/synd/inventory/agent/"
	echo >&2 
	echo >&2 "Optional arguments"
	echo >&2 "  -s	Skip 'rpm -qa' package checking (takes about 60 seconds)"
	echo >&2 "  -i	Specify file containing version of loaded os image"
	echo >&2 "  -d	Enable debugging output"
	exit 1
fi

which curl > /dev/null 2>&1
if [ 0 != $? ]; then
	echo >&2 "Error: Make sure 'curl' is installed and in your \$PATH ($PATH)"
	exit 1
fi

main() {
	xml='<?xml version="1.0" encoding="iso-8859-1"?><!DOCTYPE device PUBLIC "-//Synd//DTD Device 1.0//EN" "http://svn.synd.info/synd/branches/php4/core/module/inventory/agent/device.dtd"><device xmlns="http://www.synd.info/2005/device">'

	# Hardware info from dmidecode
	parse_dmidecode

	# Hardware info from lspci or /proc/pci
	parse_lspci
	
	# Network interfaces
	parse_ifconfig

	# IDE devices
	parse_ide

	# Monitors
	parse_ddcprobe

	# Basic operating system
	if [ -r "/etc/redhat-release" ]; then
		version=`cat /etc/redhat-release`
		release=`uname -s -r -v`
	else
		version=`uname -s -r`
		release=`uname -v`
	fi
	
	# Red Hat network serial number
	if [ -r "/etc/sysconfig/rhn/up2date-uuid" ]; then
		serial=`cat /etc/sysconfig/rhn/up2date-uuid | grep -i rhnuuid | cut -d = -f 2`
	else
		serial=""
	fi
	
	xml="$xml<os name=\"`uname -o`\" version=\"$version\" release=\"$release\" machinename=\"`uname -n`\" serial=\"$serial\">"

	# Loaded OS image
	if [ -n "$loadedimage" ]; then
		xml="$xml<image>`cat $loadedimage`</image>"
	fi

	# Installed packages
	which rpm > /dev/null 2>&1
	if [ 0 == $? ] && [ -z "$skipsoftware" ]; then
		IFS="
"
		name=""; version=""; vendor=""; release=""; installdate=""
		for line in `rpm -qai | grep -A3 Name`; do
			case $line in
				"Name"*)
					name=`echo "$line" | cut -d : -f 2 | cut -d " " -f 2`
					;;
				"Version"*)
					version=`echo "$line" | cut -d : -f 2 | cut -d " " -f 2`
					vendor=`echo "$line" | cut -d : -f 3`
					;;
				"Release"*)
					release=`echo "$line" | cut -d : -f 2 | cut -d " " -f 2`
					;;
				"Install Date"*)
					installdate=`echo "$line" | cut -d : -f 2- | cut -d : -f -3 | tr -s " " | cut -d " " -f -8`
					;;
				"--"*)
					if [ -n "$name" ] && [ -n "$version" ]; then
						xml="$xml<product vendor=\"${vendor:1}\" name=\"$name\" version=\"$version\" release=\"$release\" installdate=\"${installdate:1}\" />"
						name=""; version=""; vendor=""; release=""; installdate=""
					fi
					;;
			esac
		done
	fi

	xml="$xml</os></device>"

	# Send XML to server
	if [ -z "$debug" ]; then
		curl -f -s -k -H "Content-Type: text/xml" --data-binary "$xml" "$1" > /dev/null
	else
		echo $xml
		curl -s -k -H "Content-Type: text/xml" --data-binary "$xml" "$1"
	fi

	if [ 0 != $? ]; then
		echo >&2 "Error: Invalid response from server at '$1' (-d argument for debugging)"
		exit 1
	fi
}

# Main dmidecode parser (simple state-machine)
parse_dmidecode() {
	which dmidecode > /dev/null 2>&1
	if [ 0 != $? ]; then
		return 1
	fi

	# Fall back on sudo if not root
	if [ "0" == `id -u` ]; then
		dmidec="dmidecode"
	else
		dmidec="sudo dmidecode"
	fi

	IFS="
"
	for line in `$dmidec`; do
		case $line in
			*"Base Board Information")
				func=dmidecode_state_motherboard
				;;
			*"Chassis Information")
				func=dmidecode_state_chassis
				;;
			*"Processor Information")
				func=dmidecode_state_cpu
				;;
			*)
				if [ -n "$func" ]; then
					$func
					if [ 1 == $? ]; then
						func=""
					fi
				fi
				;;
		esac
	done
}

dmidecode_state_motherboard() {
	case $line in
		*"Manufacturer: "*)
			vendor=`echo "$line" | cut -d : -f 2`
			;;
		*"Product Name: "*)
			version=`echo "$line" | cut -d : -f 2`
			;;
		*"Version: "*)
			release=`echo "$line" | cut -d : -f 2`
			;;
		*"Serial Number: "*)
			serial=`echo "$line" | grep -v xxxxxx | cut -d : -f 2`
			;;
		"Handle "*)
			if [ -n "${vendor:1}" ]; then
				xml="$xml<motherboard vendor=\"${vendor:1}\" version=\"${version:1}\" release=\"${release:1}\" serial=\"${serial:1}\">"
				vendor=""; version=""; release=""; serial=""

				# Parse BIOS, RAM, port and slot information
				func=""
				for line in `$dmidec`; do
					case $line in
						*"BIOS Information")
							func=dmidecode_state_bios
							;;
						*"Memory Device")
							func=dmidecode_state_ram
							;;
						*"Port Connector Information")
							func=dmidecode_state_port
							;;
						*"System Slot Information")
							func=dmidecode_state_slot
							;;
						*)
							if [ -n "$func" ]; then
								$func
								if [ 1 == $? ]; then
									func=""
								fi
							fi
							;;
					esac
				done
				
				xml="$xml</motherboard>"
			fi
			vendor=""; version=""; release=""; serial=""
			return 1
			;;
	esac
}

dmidecode_state_bios() {
	case $line in
		*"Vendor: "*)
			vendor=`echo "$line" | cut -d : -f 2`
			;;
		*"Version: "*)
			version=`echo "$line" | cut -d : -f 2`
			;;
		*"Release Date: "*)
			release=`echo "$line" | cut -d : -f 2`
			;;
		"Handle "*)
			if [ -n "${vendor:1}" ]; then
				xml="$xml<bios vendor=\"${vendor:1}\" version=\"${version:1}\" release=\"${release:1}\" />"
			fi
			vendor=""; version=""; release=""
			return 1
			;;
	esac
}

dmidecode_state_ram() {
	case $line in
		*"Locator: "*)
			name=`echo "$line" | cut -d : -f 2`
			;;
		*"Type: "*)
			media=`echo "$line" | cut -d : -f 2`
			if [ "Unknown" == "${media:1:8}" ]; then
				media=""
			fi
			;;
		*"Bank Locator: "*)
			bank=`echo "$line" | cut -d : -f 2`
			;;
		*"Size: "*)
			size=`echo "$line" | cut -d : -f 2 | cut -d " " -f 2`
			if [ 'No' == "${size:0:2}" ]; then
				size=""
			fi
			;;
		*"Speed: "*)
			frequency=`echo "$line" | cut -d : -f 2 | cut -d " " -f 2`
			if [ "Unknown" == "${frequency:0:7}" ]; then
				frequency=""
			fi
			;;
		"Handle "*)
			xml="$xml<ram name=\"${name:1}\" type=\"${media:1}\" bank=\"${bank:1}\" size=\"$size\" frequency=\"$frequency\" />"
			size=""; media=""; bank=""; name=""; frequency=""
			return 1
			;;
	esac
}

dmidecode_state_port() {
	local ext
	case $line in
		*"Internal Reference Designator: "*)
			name=`echo "$line" | cut -d : -f 2`
			if [ "Other" == "${name:1}" ] || [ "None" == "${name:1}" ]; then
				name=""
			fi
			;;
		*"External Reference Designator: "*)
			ext=`echo "$line" | cut -d : -f 2 | cut -d " " -f 2`
			if [ "Other" != "$ext" ] && [ "No" != "${ext:0:2}" ] && [ -n "$ext" ]; then
				name=`echo "$line" | cut -d : -f 2`
			fi
			;;
		*"Internal Connector Type: "*)
			connector=`echo "$line" | cut -d : -f 2`
			if [ "Other" == "${connector:1}" ] || [ "None" == "${connector:1}" ]; then
				connector=""
			fi
			;;
		*"External Connector Type: "*)
			ext=`echo "$line" | cut -d : -f 2 | cut -d " " -f 2`
			if [ "Other" != "$ext" ] && [ "None" != "$ext" ]; then
				connector=`echo "$line" | cut -d : -f 2`
			fi
			;;
		*"Port Type: "*)
			media=`echo "$line" | cut -d : -f 2`
			if [ "Other" == "${media:1}" ]; then
				media=""
			fi
			;;
		"Handle "*)
			if [ -n "${name:1}" ]; then
				xml="$xml<port name=\"${name:1}\" type=\"${media:1}\" connector=\"${connector:1}\" />"
			fi
			name=""; connector=""; media=""
			return 1
			;;
	esac
}

dmidecode_state_slot() {
	local used
	case $line in
		*"Designation: "*)
			name=`echo "$line" | cut -d : -f 2`
			;;
		*"Type: "*)
			media=`echo "$line" | cut -d : -f 2`
			;;
		*"Current Usage: "*)
			used=`echo "$line" | cut -d : -f 2 | grep -i "In Use"`
			if [ 0 == $? ]; then
				occupied="1"
			else
				occupied="0"
			fi
			;;
		"Handle "*)
			if [ -n "${name:1}" ]; then
				xml="$xml<slot name=\"${name:1}\" type=\"${media:1}\" occupied=\"$occupied\" />"
			fi
			name=""; media=""; occupied=""
			return 1
			;;
	esac
}

dmidecode_state_chassis() {
	case $line in
		*"Type: "*)
			media=`echo "$line" | cut -d : -f 2`
			;;
		*"Manufacturer: "*)
			vendor=`echo "$line" | cut -d : -f 2`
			if [ "Manufactory Name" == "${vendor:1}" ]; then
				vendor=""
			fi
			;;
		*"Version: "*)
			version=`echo "$line" | grep -v "Version xx" | cut -d : -f 2`
			;;
		*"Serial Number: "*)
			serial=`echo "$line" | grep -v xxxxxx | cut -d : -f 2`
			;;
		*"Asset Tag: "*)
			dtag=`echo "$line" | grep -v xxxxxx | cut -d : -f 2`
			;;
		"Handle "*)
			if [ -n "${media:1}" ]; then
				xml="$xml<chassis type=\"${media:1}\" vendor=\"${vendor:1}\" version=\"${version:1}\" serial=\"${serial:1}\" tag=\"${dtag:1}\" />"
			fi
			media=""; vendor=""; version=""; serial=""; dtag=""
			return 1
			;;
	esac
}

dmidecode_state_cpu() {
	case $line in
		*"Manufacturer: "*)
			vendor=`echo "$line" | cut -d : -f 2`
			;;
		*"Version: "*)
			version=`echo "$line" | cut -d : -f 2`
			;;
		*"ID: "*)
			serial=`echo "$line" | cut -d : -f 2`
			;;
		*"Current Speed: "*)
			frequency=`echo "$line" | cut -d : -f 2 | cut -d " " -f 2`
			;;
		"Handle "*)
			if [ -n "${version:1}" ]; then
				xml="$xml<cpu vendor=\"${vendor:1}\" version=\"${version:1}\" frequency=\"$frequency\" serial=\"${serial// /}\" />"
			fi
			vendor=""; version=""; frequency=""; serial=""
			return 1
			;;
	esac
}

parse_ddcprobe() {
	local ddcprb name width height version size
	IFS="
"
	
	which ddcprobe > /dev/null 2>&1
	if [ 0 != $? ]; then
		return 1
	fi

	# Fall back on sudo if not root
	if [ "0" == `id -u` ]; then
		ddcprb="ddcprobe"
	else
		ddcprb="sudo ddcprobe"
	fi

	for line in `$ddcprb`; do
		case $line in
			*"Name: "*)
				name=`echo "$line" | cut -d : -f 2`
				;;
			*"Width"*)
				width=`echo "$line" | cut -d : -f 2 | cut -d " " -f 2`
				;;
			*"Height"*)
				height=`echo "$line" | cut -d : -f 2 | cut -d " " -f 2`
				;;
			*"ID: "*)
				version=`echo "$line" | cut -d : -f 2`
				size="";
				which bc > /dev/null 2>&1
				if [ 0 == $? ] && [ -n "$width" ] && [ -n "$height" ]; then
					size=`echo "sqrt(($width*$width) + ($height*$height)) / 25.4" | bc -l`;
				fi
				xml="$xml<monitor size=\"${size:0:5}\" vendorid=\"${version:1}\">${name:1}</monitor>"
				name=""; width=""; height=""; version=""; size=""
				;;
		esac
	done
}

parse_lspci() {
	local listpci line i=0 vga audio
	nics=()
	
	IFS="
"

	which lspci > /dev/null 2>&1
	if [ 0 == $? ]; then
		listpci="lspci"
	elif [ -d "/proc/pci" ]; then
		listpci="cat /proc/pci"
	else
		return 1
	fi
	
	for line in `$listpci`; do
		case $line in
			*"Ethernet controller: "*)
				nics[$i]=`echo "$line" | grep -o "Ethernet controller: .*" | cut -d : -f 2`
				let i++
				;;
			*"VGA compatible controller: "*)
				vga=`echo "$line" | grep -o "VGA compatible controller: .*" | cut -d : -f 2`
				xml="$xml<videocard version=\"${vga:1}\" />"
				;;
			*"Multimedia audio controller: "*)
				audio=`echo "$line" | grep -o "Multimedia audio controller: .*" | cut -d : -f 2`
				xml="$xml<soundcard version=\"${audio:1}\" />"
				;;
		esac
	done
}

parse_ifconfig() {
	local ifcfg interface i=0 j mac ip broadcast netmask
	IFS="
"
	
	ifcfg="ifconfig"
	which ifconfig > /dev/null 2>&1
	if [ 0 != $? ]; then
		if [ -e "/sbin/ifconfig" ]; then
			ifcfg="/sbin/ifconfig"
		else
			return 1
		fi
	fi
	
	for nic in `$ifcfg | grep -o "eth[0-9]\+ " | cut -d " " -f 1`; do
		mac=`$ifcfg $nic | grep -o "HWaddr.*" | cut -d " " -f 2`
		xml="$xml<nic mac=\"$mac\" version=\"${nics[$i]}\">"
		let i++
		
		# Add configured interfaces
		j=0
		interface=$nic
		$ifcfg | grep $interface > /dev/null
		
		while [ 0 == $? ]; do
			ip=`$ifcfg $interface | grep "inet addr" | cut -d : -f 2 | cut -d " " -f 1`
			netmask=`$ifcfg $interface | grep "inet addr" | cut -d : -f 4`
			broadcast=`$ifcfg $interface | grep "inet addr" | cut -d : -f 3 | cut -d " " -f 1`
			xml="$xml<interface name=\"$interface\" ip=\"$ip\" netmask=\"$netmask\" broadcast=\"$broadcast\" />"
			
			interface="$nic:$j"
			let j++
			
			$ifcfg | grep $interface > /dev/null
		done
		
		xml="$xml</nic>"
	done
}

parse_ide() {
	local device version size cache name media
	IFS="
"
	
	if [ ! -r "/proc/ide" ]; then
		return 1
	fi

	for device in `ls /proc/ide`; do
		if [ -r "/proc/ide/$device/media" ]; then
			case `cat /proc/ide/$device/media` in
				"disk")
					version=`cat /proc/ide/$device/model`
					size=`cat /proc/ide/$device/capacity`
					if [ -n "$size" ]; then
						let size=$size/2048
					fi
					cache=`cat /proc/ide/$device/cache`
					xml="$xml<disk name=\"$device\" version=\"$version\" size=\"$size\" cache=\"$cache\">"
					
					# Detect partion sizes and filesystems
					if [ -r /proc/partitions ]; then
						for line in `cat /proc/partitions | grep $device[0-9] | tr -s " "`; do
							name=`echo "$line" | cut -d " " -f 5`
							size=`echo "$line" | cut -d " " -f 4`
							if [ -n "$size" ]; then
								let size=$size*1024/1000/1000
							fi
							
							which fdisk > /dev/null 2>&1
							if [ 0 == $? ]; then
								media=`fdisk -l /dev/$device | grep $name | tr -s " " | cut -d " " -f 6-`
							fi
						
							xml="$xml<partition name=\"$name\" size=\"$size\" type=\"$media\" />"
						done
					fi
					
					xml="$xml</disk>"
					;;
				"cdrom")
					version=`cat /proc/ide/$device/model`
					xml="$xml<rom name=\"$device\" version=\"$version\" />"
					;;
			esac
		fi
	done
}

main $@
