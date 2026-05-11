# Enter three messages
./motd_client localhost "Message one"
./motd_client localhost "Message two"
./motd_client localhost "Message three"

# List them.
./motd_client localhost

# Add one more
./motd_client localhost "Message four"

# We shuld lose the first message entered and keep
# the most recent three.
./motd_client localhost
