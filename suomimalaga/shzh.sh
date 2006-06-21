#!/bin/bash

# Suomi-malaga, suomen kielen muoto-opin kuvaus.
#
# Tekijänoikeus © 2006 Hannu Väisänen (Etunimi.Sukunimi@joensuu.fi)
#
# Tämä ohjelma on vapaa; tätä ohjelmaa on sallittu levittää
# edelleen ja muuttaa GNU yleisen lisenssin (GPL lisenssin)
# ehtojen mukaan sellaisina kuin Free Software Foundation
# on ne julkaissut; joko Lisenssin version 2, tai (valinnan
# mukaan) minkä tahansa myöhemmän version mukaisesti.
#
# Tätä ohjelmaa levitetään siinä toivossa, että se olisi
# hyödyllinen, mutta ilman mitään takuuta; ilman edes
# hiljaista takuuta kaupallisesti hyväksyttävästä laadusta tai
# soveltuvuudesta tiettyyn tarkoitukseen. Katso GPL
# lisenssistä lisää yksityiskohtia.
#
# Tämän ohjelman mukana pitäisi tulla kopio GPL
# lisenssistä; jos näin ei ole, kirjoita osoitteeseen Free
# Software Foundation Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
# Tämän ohjeman linkittäminen staattisesti tai dynaamisesti
# muihin moduuleihin on ohjelmaan perustuvan teoksen
# tekemistä, joka on siis GPL lisenssin ehtojen alainen.
#
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to the
# Free Software Foundation, Inc., 59 Temple Place -  Suite 330, Boston, MA
# 02111-1307 USA.
#
# Linking this program statically or dynamically with other modules is
# making a combined work based on this program.  Thus, the terms and
# conditions of the GNU General Public License cover the whole
# combination.

# Muutetaan š s:ksi ja sh:ksi sekä ž z:ksi ja zh:ksi
# sanojen alkuosassa, mutta ei perusmuodossa.

cat sanat/sukija.copyright >sanat/shzh.lex

echo '' >>sanat/shzh.lex
echo '# Ohjelma shzh.sh on tehnyt tämän tiedoston.' >>sanat/shzh.lex
echo '# Älä muuta tätä.' >>sanat/shzh.lex
echo '' >>sanat/shzh.lex

grep -ih 'alku:.*[šž]' sanat/*lex \
| sed -e "s/\(alku:.*\)\š/\1s/g" \
      -e "s/\(alku:.*\)\Š/\1S/g" \
      -e "s/\(alku:.*\)\ž/\1z/g" \
      -e "s/\(alku:.*\)\Ž/\1Z/g" >>sanat/shzh.lex

grep -ih 'alku:.*[šž]' sanat/*lex \
| sed -e "s/\(alku:.*\)\š/\1sh/g" \
      -e "s/\(alku:.*\)\Š/\1Sh/g" \
      -e "s/\(alku:.*\)\ž/\1zh/g" \
      -e "s/\(alku:.*\)\Ž/\1Zh/g">>sanat/shzh.lex
