#!/usr/bin/perl -w
#
# for NP34 calculator project, zconv.pl is used to convert nonpareil code to key maps
#

use strict;

sub gen_keymap {
    my @tm = (
        25,26,27,14,23,
        29,30,31,15,13,
        33,34,35,17,0,
        37,38,39,18,0,
        1,2,3,19,0,
        5,6,7,41,0,
        9,10,11,42,0,
    );
    my @map = (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,);

    # ../../nonpareil/ncd/25-25c/25.ncd.tmpl
    my @files = `ls ../../nonpareil/*/*/[23]*.ncd.tmpl`;
    print @files;
    chomp @files;
    my @all_roms = ();
    my %ram = ();
    my %slide = ();
    my %part = ();

    for my $f (@files) {
        $f =~ m/35/ and next;
        my $rom = "xx";
        $f =~ m/\/([^\/]+)\.ncd/ and $rom = $1;
        $ram{$rom} = 0;
        $slide{$rom} = "";
        $part{$rom} = 0;
        open I, "< $f" and do {
            while (<I>) {
                /user_keycode="(\d)(\d)" hw_keycode="(\d+)"/ and do {
                    my ($p, $v) = ($tm[($1-1)*5+($2-1)], oct($3));
                    $map[$p] = $v;
                };
                /memory addr_space="data" base_addr="\d+" size="(\d+)"/ and $ram{$rom} += $1;
                /<switch_pos position="\d">  <!-- "([^"]+)" -->/ and $slide{$rom} .= $1;
                /<part_number>(\d\d\d\d)-(\d\d\d\d)/ and $part{$rom} = "$1_$2";
            }
            close I;
            push @all_roms, $rom;
            #my $dummy = <<'NOT_USED';
            print "static const uint8_t key_map_${rom}[] = {\n";
            my $i = 0;
            for (@map) {
                $i % 10 or print "\t";
                printf("0x%02x, ", $_);
                ++$i % 10 or print "\n";
            }
            print "\n};\n";
            #NOT_USED
            $f =~ s/ncd\.tmpl/asm/ and do {
                my $inc = `grep include $f`;
                $inc =~ m/(\d\d\d\d)-(\d\d\d\d)/ and $part{$rom} = "$1_$2";
            };
#{ rom_34c, rom_34c_ex, rom_1820-2105, key_map_3x, prg34c, "34C", "PGMRUN", 32, 4, 1, },
            printf "  { rom_%s, rom_%s_ex, rom_%s, rom_%s_ex, key_map_%dx, prg34c, \"%s\", \"%s\", %d, 2, %d, },\n",
              $rom, $rom, $part{$rom}, $part{$rom}, substr($rom, 0, 1), uc $rom, $slide{$rom}, oct($ram{$rom}), substr($rom, 0, 1) == '3' ? 1 : 0;
              #print "<<< $rom RAM used $ram{$rom}, $slide{$rom} >>>\n";
        };
    }
    print join(',', @all_roms), "\n";
}

sub gen_rom {
    # 19-29 need hand stitching as they have 3 rom segments
    # ../../nonpareil/build/ncd/25-25c/25.obj
    #my @files = `ls ../../nonpareil/build/ncd/*/[23]*.obj ../../nonpareil/build/ncd/*/1820*.obj`;
    my @files = `ls ../../nonpareil/build/ncd/*/19*.obj`;
    my @all_roms = ();
    #print @files;
    chomp @files;

    for my $f (@files) {
        $f =~ m/35/ and next;
        open I, "< $f" and do {
            my $rom = "xx";
            $f =~ m/\/([^\/]+)\.obj/ and $rom = $1;
            #$f =~ s/\.obj/\.lst/ and print `grep include $f`;
            $rom =~ s/-/_/;
            print "static const uint8_t rom_${rom}[] = {\n";
            my ($i, @exa, $ex) = (0, (), 0);
            while (<I>) {
                /(\d+):(\d+)/ and do {
                    my $v = oct($2);
                    $i % 16 or print "\t";
                    defined $ex or $ex = 0;
                    $ex >>= 2;
                    $ex |= ($v & 0x03) << 6;
                    $ex &= 0xff;
                    $v >>= 2;
                    printf "0x%02x, ", $v;
                    ++$i % 16 or print "\n";
                    $i % 4 or do push @exa, $ex;
                };
            }
            close I;
            print "\n};   // $i elements\n\n";
            push @all_roms, "// rom_${rom} $i elements\n";
            $i = 0;
            print "static const uint8_t rom_${rom}_ex[] = {\n";
            shift @exa;
            for my $e (@exa) {
                $i % 16 or print "\t";
                printf "0x%02x, ", $e;
                ++$i % 16 or print "\n";
            }
            print "\n};   // $i elements\n";
        };
    }
    #print @all_roms;
}

#gen_keymap;
gen_rom;

