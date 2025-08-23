{
  "version": "1.0",
  "type": "SproutBlueprint",
  "nodes": [
    {
      "id": 1,
      "type": "Event",
      "name": "OnStart",
      "position": [50, 50],
      "params": ["", "", ""],
      "inputPins": [],
      "outputPins": [101]
    },
    {
      "id": 2,
      "type": "Function",
      "name": "Print",
      "position": [250, 50],
      "params": ["Blueprint working!", "", ""],
      "inputPins": [201],
      "outputPins": [202]
    }
  ],
  "connections": [
    {"from": 101, "to": 201}
  ]
}
