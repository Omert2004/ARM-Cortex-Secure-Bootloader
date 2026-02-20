from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend


def fmt_bytes(data, indent='    ', cols=8):
    """Format bytes as C hex literals, `cols` per row."""
    rows = []
    for i in range(0, len(data), cols):
        chunk = data[i:i + cols]
        rows.append(indent + ', '.join(f'0x{b:02X}' for b in chunk))
    return ',\n'.join(rows)


# --- 1. Extract ECDSA Public Key (X || Y concatenated) ---
try:
    with open("public.pem", "rb") as f:
        public_key = serialization.load_pem_public_key(f.read(), backend=default_backend())

    numbers = public_key.public_numbers()
    x = numbers.x.to_bytes(32, byteorder='big')
    y = numbers.y.to_bytes(32, byteorder='big')

    # Output as a single concatenated array matching ECDSA_public_key_xy[64] in keys.c
    print("/* ECDSA Public Key (from public.pem) */")
    print("const uint8_t ECDSA_public_key_xy[64] = {")
    print("    /* X Coordinate */")
    print(fmt_bytes(x) + ",")
    print()
    print("    /* Y Coordinate */")
    print(fmt_bytes(y))
    print("};")

except FileNotFoundError:
    print("Error: 'public.pem' not found. Run keygen.py first.")

# --- 2. Extract AES Secret Key ---
try:
    with open("secret.key", "rb") as f:
        aes_key = f.read()

    print(f"\n/* AES Secret Key (from secret.key) */")
    print(f"const uint8_t AES_SECRET_KEY[{len(aes_key)}] = {{")
    print(fmt_bytes(aes_key))
    print("};")

except FileNotFoundError:
    print("\nError: 'secret.key' not found. Run keygen.py first.")
