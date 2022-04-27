#!/bin/bash

# Name: t.colors.sh
# Author: Ladar Levison
#
# Description: Used for testing the supported colors and printing the associated ANSI codes, or tput commands.

echo
echo "$(tput bold)    ANSI COLORS$(tput sgr0)"

tput sgr0 ; printf "\n\e[0;1m      Text Modifiers\n\n" ; tput sgr0
printf "      T_BOLD=\"%s\"        \e[0;1m# SAMPLE TEXT\e[0;0m\n" "\e[0;1m"
printf "      T_ULINE=\"%s\"       \e[0;4m# SAMPLE TEXT\e[0;0m\n" "\e[0;4m"
printf "      T_STANDOUT=\"%s\"    \e[0;7m# SAMPLE TEXT\e[0;0m\n" "\e[0;7m"
printf "      T_RESET=\"%s\"       \e[0;0m# SAMPLE TEXT\e[0;0m\n" "\e[0;0m"

tput sgr0 ; printf "\n\e[0;1m      Foreground Colors\n\n" ; tput sgr0
printf "      T_BLK=\"%s\"        \e[0;30m# SAMPLE TEXT\e[0;0m\n" "\e[0;30m"
printf "      T_RED=\"%s\"        \e[0;31m# SAMPLE TEXT\e[0;0m\n" "\e[0;31m"
printf "      T_GRN=\"%s\"        \e[0;32m# SAMPLE TEXT\e[0;0m\n" "\e[0;32m"
printf "      T_YEL=\"%s\"        \e[0;33m# SAMPLE TEXT\e[0;0m\n" "\e[0;33m"
printf "      T_BLU=\"%s\"        \e[0;34m# SAMPLE TEXT\e[0;0m\n" "\e[0;34m"
printf "      T_MAG=\"%s\"        \e[0;35m# SAMPLE TEXT\e[0;0m\n" "\e[0;35m"
printf "      T_CYN=\"%s\"        \e[0;36m# SAMPLE TEXT\e[0;0m\n" "\e[0;36m"
printf "      T_WHT=\"%s\"        \e[0;37m# SAMPLE TEXT\e[0;0m\n" "\e[0;37m"

tput sgr0 ; printf "\n\e[0;1m      Foreground Colors (With Bold)\n\n" ; tput sgr0
printf "      T_BBLK=\"%s\"       \e[1;30m# SAMPLE TEXT\e[0;0m\n" "\e[1;30m"
printf "      T_BRED=\"%s\"       \e[1;31m# SAMPLE TEXT\e[0;0m\n" "\e[1;31m"
printf "      T_BGRN=\"%s\"       \e[1;32m# SAMPLE TEXT\e[0;0m\n" "\e[1;32m"
printf "      T_BYEL=\"%s\"       \e[1;33m# SAMPLE TEXT\e[0;0m\n" "\e[1;33m"
printf "      T_BBLU=\"%s\"       \e[1;34m# SAMPLE TEXT\e[0;0m\n" "\e[1;34m"
printf "      T_BMAG=\"%s\"       \e[1;35m# SAMPLE TEXT\e[0;0m\n" "\e[1;35m"
printf "      T_BCYN=\"%s\"       \e[1;36m# SAMPLE TEXT\e[0;0m\n" "\e[1;36m"
printf "      T_BWHT=\"%s\"       \e[1;37m# SAMPLE TEXT\e[0;0m\n" "\e[1;37m"

tput sgr0 ; printf "\n\e[0;1m      Foreground Colors (With Underline)\n\n" ; tput sgr0
printf "      T_UBLK=\"%s\"       \e[4;30m# SAMPLE TEXT\e[0;0m\n" "\e[4;30m"
printf "      T_URED=\"%s\"       \e[4;31m# SAMPLE TEXT\e[0;0m\n" "\e[4;31m"
printf "      T_UGRN=\"%s\"       \e[4;32m# SAMPLE TEXT\e[0;0m\n" "\e[4;32m"
printf "      T_UYEL=\"%s\"       \e[4;33m# SAMPLE TEXT\e[0;0m\n" "\e[4;33m"
printf "      T_UBLU=\"%s\"       \e[4;34m# SAMPLE TEXT\e[0;0m\n" "\e[4;34m"
printf "      T_UMAG=\"%s\"       \e[4;35m# SAMPLE TEXT\e[0;0m\n" "\e[4;35m"
printf "      T_UCYN=\"%s\"       \e[4;36m# SAMPLE TEXT\e[0;0m\n" "\e[4;36m"
printf "      T_UWHT=\"%s\"       \e[4;37m# SAMPLE TEXT\e[0;0m\n" "\e[4;37m"

tput sgr0 ; printf "\n\e[0;1m      Background Colors\n\n" ; tput sgr0
printf "      T_BLKB=\"%s\"         \e[40m# SAMPLE TEXT\e[0;0m\n" "\e[40m"
printf "      T_REDB=\"%s\"         \e[41m# SAMPLE TEXT\e[0;0m\n" "\e[41m"
printf "      T_GRNB=\"%s\"         \e[42m# SAMPLE TEXT\e[0;0m\n" "\e[42m"
printf "      T_YELB=\"%s\"         \e[43m# SAMPLE TEXT\e[0;0m\n" "\e[43m"
printf "      T_BLUB=\"%s\"         \e[44m# SAMPLE TEXT\e[0;0m\n" "\e[44m"
printf "      T_MAGB=\"%s\"         \e[45m# SAMPLE TEXT\e[0;0m\n" "\e[45m"
printf "      T_CYNB=\"%s\"         \e[46m# SAMPLE TEXT\e[0;0m\n" "\e[46m"
printf "      T_WHTB=\"%s\"         \e[47m# SAMPLE TEXT\e[0;0m\n" "\e[47m"

tput sgr0 ; printf "\n\e[0;1m      Background Colors (High Intensity)\n\n" ; tput sgr0
printf "      T_BLKHB=\"%s\"     \e[0;100m# SAMPLE TEXT\e[0;0m\n" "\e[0;100m"
printf "      T_REDHB=\"%s\"     \e[0;101m# SAMPLE TEXT\e[0;0m\n" "\e[0;101m"
printf "      T_GRNHB=\"%s\"     \e[0;102m# SAMPLE TEXT\e[0;0m\n" "\e[0;102m"
printf "      T_YELHB=\"%s\"     \e[0;103m# SAMPLE TEXT\e[0;0m\n" "\e[0;103m"
printf "      T_BLUHB=\"%s\"     \e[0;104m# SAMPLE TEXT\e[0;0m\n" "\e[0;104m"
printf "      T_MAGHB=\"%s\"     \e[0;105m# SAMPLE TEXT\e[0;0m\n" "\e[0;105m"
printf "      T_CYNHB=\"%s\"     \e[0;106m# SAMPLE TEXT\e[0;0m\n" "\e[0;106m"
printf "      T_WHTHB=\"%s\"     \e[0;107m# SAMPLE TEXT\e[0;0m\n" "\e[0;107m"

tput sgr0 ; printf "\n\e[0;1m      Foreground Colors (High Intensity)\n\n" ; tput sgr0
printf "      T_HBLK=\"%s\"       \e[0;90m# SAMPLE TEXT\e[0;0m\n" "\e[0;90m"
printf "      T_HRED=\"%s\"       \e[0;91m# SAMPLE TEXT\e[0;0m\n" "\e[0;91m"
printf "      T_HGRN=\"%s\"       \e[0;92m# SAMPLE TEXT\e[0;0m\n" "\e[0;92m"
printf "      T_HYEL=\"%s\"       \e[0;93m# SAMPLE TEXT\e[0;0m\n" "\e[0;93m"
printf "      T_HBLU=\"%s\"       \e[0;94m# SAMPLE TEXT\e[0;0m\n" "\e[0;94m"
printf "      T_HMAG=\"%s\"       \e[0;95m# SAMPLE TEXT\e[0;0m\n" "\e[0;95m"
printf "      T_HCYN=\"%s\"       \e[0;96m# SAMPLE TEXT\e[0;0m\n" "\e[0;96m"
printf "      T_HWHT=\"%s\"       \e[0;97m# SAMPLE TEXT\e[0;0m\n" "\e[0;97m"

tput sgr0 ; printf "\n\e[0;1m      Foreground Colors (High Intensity With Bold)\n\n" ; tput sgr0
printf "      T_BHBLK=\"%s\"      \e[1;90m# SAMPLE TEXT\e[0;0m\n" "\e[1;90m"
printf "      T_BHRED=\"%s\"      \e[1;91m# SAMPLE TEXT\e[0;0m\n" "\e[1;91m"
printf "      T_BHGRN=\"%s\"      \e[1;92m# SAMPLE TEXT\e[0;0m\n" "\e[1;92m"
printf "      T_BHYEL=\"%s\"      \e[1;93m# SAMPLE TEXT\e[0;0m\n" "\e[1;93m"
printf "      T_BHBLU=\"%s\"      \e[1;94m# SAMPLE TEXT\e[0;0m\n" "\e[1;94m"
printf "      T_BHMAG=\"%s\"      \e[1;95m# SAMPLE TEXT\e[0;0m\n" "\e[1;95m"
printf "      T_BHCYN=\"%s\"      \e[1;96m# SAMPLE TEXT\e[0;0m\n" "\e[1;96m"
printf "      T_BHWHT=\"%s\"      \e[1;97m# SAMPLE TEXT\e[0;0m\n" "\e[1;97m"

echo
echo "$(tput bold)    TPUT COLORS$(tput sgr0)"

# Text color variables
txtund=$(tput sgr 0 1)              # Underline
txtbld=$(tput bold)                   # Bold
bldred=${txtbld}$(tput setaf 1) #  Red
bldblu=${txtbld}$(tput setaf 4) #  Blue
bldwht=${txtbld}$(tput setaf 7) #  White
txtrst=$(tput sgr0)                   # Reset
info=${bldwht}*${txtrst}            # Feedback
pass=${bldblu}*${txtrst}
warn=${bldred}!${txtrst}

echo
echo "$(tput bold)      Text Modifiers$(tput sgr0)"
echo
echo '      Bold                    $(tput bold)'
echo '      Underline               $(tput sgr 0 1)'
echo '      Move Up / Delete        $(tput cuu1 ; tput dl1)'
echo '      Reset                   $(tput sgr0)'

echo
echo "$(tput bold)      Foreground Colors$(tput sgr0)"
echo
echo -e "$(tput bold)      reg  bld  und$(tput sgr0)"

for i in $(seq 1 7); do
  echo "      $(tput setaf $i)Text$(tput sgr0) $(tput bold)$(tput setaf $i)Text$(tput sgr0) $(tput sgr 0 1)$(tput setaf $i)Text$(tput sgr0)          \$(tput setaf $i)"
done

echo
echo "$(tput bold)      Background Colors$(tput sgr0)"
echo
echo -e "$(tput bold)      reg  bld  und$(tput sgr0)"

for i in $(seq 1 7); do
  echo "      $(tput setab $i)Text$(tput sgr0) $(tput bold)$(tput setab $i)Text$(tput sgr0) $(tput sgr 0 1)$(tput setab $i)Text$(tput sgr0)          \$(tput setab $i)"
done
echo
