# LastLineProvider
Read text from standard input, line by line, and provides the most recent one through the file specified with -o option. Additionlaly, by default, each read line is being rewrited to the standard output (what can be disabled).

## Compilation

```
git clone https://github.com/komputerowiec/LastLineProvider.git
cd LastLineProvider/LastLineProvider/
cmake .
make
```

After successfull compilation you will find the binary exectable in current working directory. It is named lastlineprovider.

## Instalation

The binary executable can be used directly from the directroy where it was compiled. 
You can also copy it to the place of your choice or just install it:

```
sudo make install
```

## Usage

```
lastlineprovider -h
lastlineprovider -o <output-file-path> [-f <regex-filter>] [-l] [-n] [-m] [-s]
```

Options:

`-o <output-file-path>` <br />
Specifies the path to the output file which provides the content of the last line read from standard input.

`-f <regex-filter>` <br />
If set, only lines which match the <regex-filter> regular expression will be provided through the output file specified with -o option.

`-l` <br />
If set, before any new line is written to the `<output-file-path>` file, the file is beaing locked with the `LOCK_EX` operation of the flock(2) system call. The lock is being released after the whole line is written to the file.

`-n` <br />
No standard output, i.e. the lines read from the standard input are not copied to the standard output, so that only `<output-file-path>` file is being updated.

`-s` <br />
Sychronize each write to the `<output-file-path>`, i.e. call fsync() after each line is written.

`-m` <br />
Matching only, i.e. rewrite to the standard output copy only lines matching the `<regex-filter>` filter (or all lines if the filter is not defined).

## Examples

Every second (dstat by default returns metrics every second), left the value of current output bandwitch of eth0 network interface in the file `/var/log/curr_eth0_out`:

```
dstat -n --noheaders -N eth0 | awk '{print $2; fflush("/dev/stdout")}' | lastlineprovider -o /var/log/curr_eth0_out
```

Every second, left in the file `/var/lib/rtt_value` the current RTT value measured against IP address 8.8.8.8:

```
ping 8.8.8.8 | awk 'match($7, /[0-9]+.*/) {print substr($7, RSTART, RLENGTH); fflush("/dev/stdout")}' | lastlineprovider -o /var/lib/rtt_value
```
