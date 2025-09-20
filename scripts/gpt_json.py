import json

filename = "/mnt/e/University/Year4/gpt2/conversations.json"
conversations = {
    '4bef19e4-631d-43bb-8807-1e5c3b51f31e',
    'c2e0c06c-72fd-47fb-ae41-b9e195e11ba9',
    'bdd8a087-5dd4-4e64-855e-2467877ddec8',
    '5a913f2b-ce2e-4357-861e-32a969ff4bce',
    '7f584ee4-07a9-4e74-825d-5e6195b8ed8e',
}

features = {
    'Basic Raytracer:1': 0,
    'Basic Raytracer:2': 0,
    'Basic Raytracer:3': 0,
    'Basic Raytracer:4': 0,
    'Basic Raytracer:5': 0,
    'Basic Raytracer:6': 0,
    'Basic Raytracer:7': 0,
    'Basic Raytracer:8': 0,
    'Basic Raytracer:9': 0,
    'Basic Raytracer:10': 0,
    'Intermediate Raytracer:1': 0,
    'Intermediate Raytracer:2': 0,
    'Intermediate Raytracer:7': 0,
    'Intermediate Raytracer:11': 0,
    'Pathtracer:1': 0,
    'Pathtracer:2': 0,
    'Pathtracer:3': 0,
    'Pathtracer:4': 0,
    'Pathtracer:5': 0,
}

CONTINUE = True
if __name__ == "__main__":
    with open(filename) as f:
        data = json.load(f)
    result = []

    if CONTINUE:
        with open('llm_responses3.json') as f:
            result = json.load(f)

        for row in result:
            features[row['feature']] += 1

        last_query = result[-1]['query']
        found = False
    else:
        found = True

    i = 0
    for conv_idx in range(len(data)):
        conv = data[conv_idx]
        if (conv['id'] not in conversations):
            continue

        mapping = conv['mapping']
        root = None

        print(conv['title'], conv['id'])

        for key in list(mapping.keys()):
            if mapping[key]['parent'] is None:
                root = key
                break

        queue = [root]

        curr_question = None
        while len(queue) > 0:
            key = queue.pop(0)
            queue.extend(mapping[key]['children'])
            if len(mapping[key]['children']) > 1:
                print("ERROR: Number of Answers Exceeds Expected")

            message = mapping[key]['message']
            if message is None:
                continue

            if message['author']['role'] == 'user':
                curr_question = message['content']['parts']
                if len(message['content']['parts']) > 1:
                    print("ERROR: Number of answe parts exceeds expected")

            if message['author']['role'] != 'user':
                new_json = {}

                curr_answer = message['content']['parts'][0]

                print("Question: ", curr_question)
                print("Answer: ", curr_answer)
                if not found:
                    if curr_question == last_query:
                        found = True
                    continue

                enter = input("Enter 1 for enter and 0 otherwise: ")

                while enter != '0' and enter != '1' and enter != 's':
                    enter = input("Enter 1 for enter and 0 otherwise: ")

                if enter == '0':
                    continue

                feature = input("Enter corresponding feature: ")
                while feature not in features:
                    feature = input("Enter corresponding feature: ")

                tag = feature + "-" + "".join('0' for i in range(
                    3-len(str(features[feature])))) + str(features[feature])
                features[feature] += 1

                method = "chatgpt"
                file = input("Enter filename")
                line_start = int(input("Approx line start"))
                line_end = int(input("Approx line end"))

                new_json['feature'] = feature
                new_json['method'] = method
                new_json['query'] = curr_question
                new_json['response'] = message['content']['parts'][0]
                new_json['code'] = (file + ":" +
                                    str(line_start) + "-" + file + ":" + str(line_end))
                new_json['id'] = tag

                result.append(new_json)
                if len(message['content']['parts']) > 1:
                    print("ERROR: Number of answe parts exceeds expected")

                with open("llm_responses4.json", "w") as f:
                    json.dump(result, f)
    with open("llm_responses3.json", "w") as f:
        json.dump(result, f)
