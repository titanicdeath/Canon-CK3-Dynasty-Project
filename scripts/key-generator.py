
# key = " 0123456789+-=/\\.,[]{}AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvSsWwXxYyZz!?\"\'\b\n\r\t"

# verification = {}
# # reversedVerification = {}

# for i in range(len(key)):
#     verification[key[i]] = i




# rawKey = "{' ': 0, '0': 1, '1': 2, '2': 3, '3': 4, '4': 5, '5': 6, '6': 7, '7': 8, '8': 9, '9': 10, '+': 11, '-': 12, '=': 13, '/': 14, '\\': 15, '.': 16, ',': 17, '[': 18, ']': 19, '{': 20, '}': 21, 'A': 22, 'a': 23, 'B': 24, 'b': 25, 'C': 26, 'c': 27, 'D': 28, 'd': 29, 'E': 30, 'e': 31, 'F': 32, 'f': 33, 'G': 34, 'g': 35, 'H': 36, 'h': 37, 'I': 38, 'i': 39, 'J': 40, 'j': 41, 'K': 42, 'k': 43, 'L': 44, 'l': 45, 'M': 46, 'm': 47, 'N': 48, 'n': 49, 'O': 50, 'o': 51, 'P': 52, 'p': 53, 'Q': 54, 'q': 55, 'R': 56, 'r': 57, 'S': 66, 's': 67, 'T': 60, 't': 61, 'U': 62, 'u': 63, 'V': 64, 'v': 65, 'W': 68, 'w': 69, 'X': 70, 'x': 71, 'Y': 72, 'y': 73, 'Z': 74, 'z': 75, '!': 76, '?': 77, '"': 78, "'": 79, '\\x08': 80, '\\n': 81, '\\r': 82, '\\t': 83} "

# comma = 0
# better = ""
# for i in range(len(rawKey)):
#     cur = rawKey[i]
#     if (cur == ','):
#         comma += 1
#         if (comma == 4) and (rawKey[i-1] != '\''):
#             cur =cur + "\n\t"
#             comma = 0
#         elif (rawKey[i-1] == '\''):
#             comma -= 1
#     better = better + cur


# bodyParagraphs = "Hello my name is Alden Simmons"

# rVerLi = []
# bufferLi = []
# for i in range(len(bodyParagraphs)):
#     buffer = f"buffer[{i}]"
#     bufferLi.append(buffer)
#     rVer = f"reVer[{verification[bodyParagraphs[i]]}]"
#     rVerLi.append(rVer)

# trinary = [bodyParagraphs, bufferLi, rVerLi]
# out = []

# for i in range(len(trinary)):
#     cur = trinary[i]
#     unique = 0
#     contain = []
#     for x in range(len(cur)):
#         if (cur[x] not in contain):
#             unique += 1
#             contain.append(cur[x])
#     out.append(unique)


# print(bodyParagraphs)
# print("-"*10)
# print(f"This contains {out[0]} unique value")
# print("\n")
# print(bufferLi)
# print("-"*10)
# print(f"This contains {out[1]} unique value")
# print("\n")
# print(rVerLi)
# print("-"*10)
# print(f"This contains {out[2]} unique value")
# print("\n")

# para = "We conclude that under our constitutional structure of\
#         separated powers, the nature of Presidential power requires that a former President have some immunity from\
#         criminal prosecution for official acts during his tenure in\
#         office. At least with respect to the President's exercise of\
#         his core constitutional powers, this immunity must be absolute. As for his remaining official actions, he is also entitled to immunity. At the current stage of proceedings in\
#         this case, however, we need not and do not decide whether\
#         that immunity must be absolute, or instead whether a presumptive immunity is sufficient."

# para = "Criminally prosecuting a President for official conduct\
#         undoubtedly poses a far greater threat of intrusion on the\
#         authority and functions of the Executive Branch than\
#         simply seeking evidence in his possession, as in Burr and Nixon. The danger is akin to, indeed greater than, what led\
#         us to recognize absolute Presidential immunity from civil\
#         damages liability—that the President would be chilled from\
#         taking the “bold and unhesitating action” required of an independent Executive."

para = "Taking into account these competing considerations, we\
        conclude that the separation of powers principles explicated\
        in our precedent necessitate at least a presumptive immunity from criminal prosecution for a President's acts within\
        the outer perimeter of his official responsibility. Such an\
        immunity is required to safeguard the independence and\
        effective functioning of the Executive Branch, and to enable\
        the President to carry out his constitutional duties without\
        undue caution."

#para.strip("\n")
para.replace("\n", "")
print(para)
print(f"\n\n{len(para)}\n\n")
old = ""
loong = len(para) - 4
for i in range(loong):
    print(f"- {i} / {loong}")
    if (i < loong):
        if ((para[i] == " ") and (para[i+1] == " ")):
            para = para[:i] + para[i+3:]
            loong = loong - 3
            print(f"{i} / {len(para)} | {para[i]}")
    else:
        print(i)

print(para)
