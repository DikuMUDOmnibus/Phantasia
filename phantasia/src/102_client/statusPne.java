import java.awt.Panel;
import java.awt.Label;
import java.awt.Canvas;
import java.awt.Color;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.GridLayout;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.io.DataInputStream;
import java.awt.Font;

public class statusPne extends Panel implements constants {

    private pClient parent = null;

    GridBagLayout paneLayout = new GridBagLayout();
    GridBagLayout energyLayout = new GridBagLayout();
    GridBagLayout strengthLayout = new GridBagLayout();
    GridBagLayout quicknessLayout = new GridBagLayout();

    private Panel titlePane = new Panel();
    private headPne titleCanvas = new headPne("Welcome to Phantasia!");

    private Panel statsPane = new Panel();

    private Panel energyPane = new Panel();
    private titlePne energyLabel = new titlePne("Energy");
    private barGraph energyGraph = new barGraph();
    public imagePne shieldImage = null;
    private numberPne shieldLabel = new numberPne("0");

    private Panel strengthPane = new Panel();
    private titlePne strengthLabel = new titlePne("Strength");
    private barGraph strengthGraph = new barGraph();
    private imagePne swordImage = null;
    private numberPne swordLabel = new numberPne("0");

    private Panel quicknessPane = new Panel();
    private titlePne quicknessLabel = new titlePne("Speed");
    private barGraph quicknessGraph = new barGraph();
    private imagePne quicksilverImage = null;
    private numberPne quicksilverLabel = new numberPne("0");

    private Panel treasurePane = new Panel();

    private Panel manaPane = new Panel();
    private imagePne manaImage = null;
    private numberPne manaLabel = new numberPne("0");

    private Panel levelPane = new Panel();
    private imagePne levelImage = null;
    private numberPne levelLabel = new numberPne("0");

    private Panel goldPane = new Panel();
    private imagePne goldImage = null;
    private numberPne goldLabel = new numberPne("0");

    private Panel gemsPane = new Panel();
    private imagePne gemsImage = null;
    private numberPne gemsLabel = new numberPne("0");

    private Panel equipmentPane = new Panel();

    private imagePne cloakImage = null;
    private imagePne blessingImage = null;
    private imagePne crownImage = null;
    private imagePne palantirImage = null;
    private imagePne ringImage = null;
    private imagePne virginImage = null;

    private String name;          /* player name */
    private String location;	/* player location */
    public statusPne(pClient c) {

	GridBagConstraints constraints = new GridBagConstraints();

	parent = c;

        shieldImage = new imagePne(parent);
        swordImage = new imagePne(parent);
        quicksilverImage = new imagePne(parent);
	manaImage = new imagePne(parent);
	levelImage = new imagePne(parent);
	goldImage = new imagePne(parent);
	gemsImage = new imagePne(parent);
	cloakImage = new imagePne(parent);
	blessingImage = new imagePne(parent);
	crownImage = new imagePne(parent);
	palantirImage = new imagePne(parent);
	ringImage = new imagePne(parent);
	virginImage = new imagePne(parent);

	constraints.insets.top = 2;
	constraints.insets.bottom = 2;
	constraints.insets.left = 2;
	constraints.insets.right = 2;

	setBackground(backgroundColor);


/* START MAIN PANE LAYOUT */
	setLayout(paneLayout);

	constraints.gridx = 1;
	constraints.gridy = 0;
	constraints.gridwidth = 1;
	constraints.gridheight = 1;
	constraints.weightx = 0;
	constraints.weighty = 0;
	constraints.fill = GridBagConstraints.HORIZONTAL;
	constraints.anchor = GridBagConstraints.CENTER;
	paneLayout.setConstraints(titleCanvas, constraints);
	add(titleCanvas);

	constraints.gridx = 0;
	constraints.gridy = 0;
	constraints.gridwidth = 1;
	constraints.gridheight = 3;
	constraints.weightx = 1;
	constraints.weighty = 0;
	constraints.fill = GridBagConstraints.HORIZONTAL;
	constraints.anchor = GridBagConstraints.CENTER;
	paneLayout.setConstraints(statsPane, constraints);
	add(statsPane);

	constraints.gridx = 1;
	constraints.gridy = 1;
	constraints.gridwidth = 1;
	constraints.gridheight = 1;
	constraints.weightx = 0;
	constraints.weighty = 0;
	constraints.fill = GridBagConstraints.HORIZONTAL;
	constraints.anchor = GridBagConstraints.CENTER;
	paneLayout.setConstraints(treasurePane, constraints);
	add(treasurePane);

	constraints.gridx = 1;
	constraints.gridy = 2;
	constraints.gridwidth = 1;
	constraints.gridheight = 1;
	constraints.weightx = 0;
	constraints.weighty = 1;
	constraints.fill = GridBagConstraints.BOTH;
	constraints.anchor = GridBagConstraints.CENTER;
	paneLayout.setConstraints(equipmentPane, constraints);
	equipmentPane.setBackground(backgroundColor);
	add(equipmentPane);
/* END MAIN PANE LAYOUT */

/* START STATS PANE LAYOUT */
	statsPane.setLayout(new GridLayout(3, 1, 2, 4));
	statsPane.setBackground(backgroundColor);
	statsPane.add(energyPane);
	statsPane.add(strengthPane);
	statsPane.add(quicknessPane);

	energyPane.setLayout(energyLayout);
	energyPane.setBackground(highlightColor);
	constraints.insets.top = 0;
	constraints.insets.bottom = 0;
	constraints.insets.left = 0;
	constraints.insets.right = 0;

	constraints.gridx = 0;
	constraints.gridy = 0;
	constraints.gridwidth = 3;
	constraints.gridheight = 1;
	constraints.weightx = 0;
	constraints.weighty = 0;
	constraints.fill = GridBagConstraints.HORIZONTAL;
	constraints.anchor = GridBagConstraints.CENTER;
	energyLayout.setConstraints(energyLabel, constraints);
	energyLabel.resize(16, 16);
	energyPane.add(energyLabel);

	constraints.gridx = 0;
	constraints.gridy = 1;
	constraints.gridwidth = 1;
	constraints.gridheight = 1;
	constraints.weightx = 1;
	constraints.weighty = 0;
	constraints.fill = GridBagConstraints.BOTH;
	constraints.anchor = GridBagConstraints.CENTER;
	energyLayout.setConstraints(energyGraph, constraints);
	energyPane.add(energyGraph);

	constraints.gridx = 1;
	constraints.gridy = 1;
	constraints.gridwidth = 1;
	constraints.gridheight = 1;
	constraints.weightx = 0;
	constraints.weighty = 0;
	constraints.insets.left = 4;
	constraints.insets.right = 2;
	constraints.fill = GridBagConstraints.NONE;
	constraints.anchor = GridBagConstraints.CENTER;
	energyLayout.setConstraints(shieldImage, constraints);
	shieldImage.resize(18, 18);
	energyPane.add(shieldImage);

	constraints.gridx = 2;
	constraints.gridy = 1;
	constraints.gridwidth = 1;
	constraints.gridheight = 1;
	constraints.weightx = 0;
	constraints.weighty = 0;
	constraints.insets.left = 0;
	constraints.insets.right = 0;
	constraints.fill = GridBagConstraints.NONE;
	constraints.anchor = GridBagConstraints.CENTER;
	energyLayout.setConstraints(shieldLabel, constraints);
	energyPane.add(shieldLabel);


	strengthPane.setLayout(strengthLayout);
	strengthPane.setBackground(highlightColor);

	constraints.gridx = 0;
	constraints.gridy = 0;
	constraints.gridwidth = 3;
	constraints.gridheight = 1;
	constraints.weightx = 0;
	constraints.weighty = 0;
	constraints.fill = GridBagConstraints.HORIZONTAL;
	constraints.anchor = GridBagConstraints.CENTER;
	strengthLayout.setConstraints(strengthLabel, constraints);
	strengthLabel.resize(16, 16);
	strengthPane.add(strengthLabel);

	constraints.gridx = 0;
	constraints.gridy = 1;
	constraints.gridwidth = 1;
	constraints.gridheight = 1;
	constraints.weightx = 1;
	constraints.weighty = 0;
	constraints.fill = GridBagConstraints.BOTH;
	constraints.anchor = GridBagConstraints.CENTER;
	strengthLayout.setConstraints(strengthGraph, constraints);
	strengthPane.add(strengthGraph);

	constraints.gridx = 1;
	constraints.gridy = 1;
	constraints.gridwidth = 1;
	constraints.gridheight = 1;
	constraints.weightx = 0;
	constraints.weighty = 0;
	constraints.insets.left = 4;
	constraints.insets.right = 2;
	constraints.fill = GridBagConstraints.NONE;
	constraints.anchor = GridBagConstraints.CENTER;
	strengthLayout.setConstraints(swordImage, constraints);
	swordImage.resize(18, 18);
	strengthPane.add(swordImage);

	constraints.gridx = 2;
	constraints.gridy = 1;
	constraints.gridwidth = 1;
	constraints.gridheight = 1;
	constraints.weightx = 0;
	constraints.weighty = 0;
	constraints.insets.left = 0;
	constraints.insets.right = 0;
	constraints.fill = GridBagConstraints.NONE;
	constraints.anchor = GridBagConstraints.CENTER;
	strengthLayout.setConstraints(swordLabel, constraints);
	strengthPane.add(swordLabel);


	quicknessPane.setLayout(quicknessLayout);
	quicknessPane.setBackground(highlightColor);

	constraints.gridx = 0;
	constraints.gridy = 0;
	constraints.gridwidth = 3;
	constraints.gridheight = 1;
	constraints.weightx = 0;
	constraints.weighty = 0;
	constraints.fill = GridBagConstraints.HORIZONTAL;
	constraints.anchor = GridBagConstraints.CENTER;
	quicknessLayout.setConstraints(quicknessLabel, constraints);
	quicknessLabel.resize(16, 16);
	quicknessPane.add(quicknessLabel);

	constraints.gridx = 0;
	constraints.gridy = 1;
	constraints.gridwidth = 1;
	constraints.gridheight = 1;
	constraints.weightx = 1;
	constraints.weighty = 0;
	constraints.fill = GridBagConstraints.BOTH;
	constraints.anchor = GridBagConstraints.CENTER;
	quicknessLayout.setConstraints(quicknessGraph, constraints);
	quicknessPane.add(quicknessGraph);

	constraints.gridx = 1;
	constraints.gridy = 1;
	constraints.gridwidth = 1;
	constraints.gridheight = 1;
	constraints.weightx = 0;
	constraints.weighty = 0;
	constraints.insets.left = 4;
	constraints.insets.right = 2;
	constraints.fill = GridBagConstraints.NONE;
	constraints.anchor = GridBagConstraints.CENTER;
	quicknessLayout.setConstraints(quicksilverImage, constraints);
	quicksilverImage.resize(18, 18);
	quicknessPane.add(quicksilverImage);

	constraints.gridx = 2;
	constraints.gridy = 1;
	constraints.gridwidth = 1;
	constraints.gridheight = 1;
	constraints.weightx = 0;
	constraints.weighty = 0;
	constraints.insets.left = 0;
	constraints.insets.right = 0;
	constraints.fill = GridBagConstraints.NONE;
	constraints.anchor = GridBagConstraints.CENTER;
	quicknessLayout.setConstraints(quicksilverLabel, constraints);
	quicknessPane.add(quicksilverLabel);
/* END STATS PANE LAYOUT */

/* BEGIN TREASUE PANE LAYOUT */
	treasurePane.setLayout(new GridLayout(2, 2, 4, 4));
	treasurePane.setBackground(backgroundColor);
	treasurePane.add(manaPane);
	treasurePane.add(goldPane);
	treasurePane.add(levelPane);
	treasurePane.add(gemsPane);

	manaPane.setLayout(new BorderLayout(1, 0));
	manaPane.setBackground(highlightColor);
	manaImage.resize(18, 18);
	manaPane.add("West", manaImage);
	manaPane.add("East", manaLabel);

	goldPane.setLayout(new BorderLayout(1, 0));
	goldPane.setBackground(highlightColor);
	goldImage.resize(18, 18);
	goldPane.add("West", goldImage);
	goldPane.add("East", goldLabel);

	levelPane.setLayout(new BorderLayout(1, 0));
	levelPane.setBackground(highlightColor);
	levelImage.resize(18, 18);
	levelPane.add("West", levelImage);
	levelPane.add("East", levelLabel);

	gemsPane.setLayout(new BorderLayout(1, 0));
	gemsPane.setBackground(highlightColor);
	gemsImage.resize(18, 18);
	gemsPane.add("West", gemsImage);
	gemsPane.add("East", gemsLabel);
/* END TREASUE PANE LAYOUT */

/* BEGIN INVENTORY PANE LAYOUT */

	equipmentPane.setLayout(new FlowLayout(FlowLayout.CENTER, 10, 0));

	cloakImage.resize(24, 24);
	equipmentPane.add(cloakImage);

	blessingImage.resize(24, 24);
	equipmentPane.add(blessingImage);

	crownImage.resize(24, 24);
	equipmentPane.add(crownImage);

	palantirImage.resize(24, 24);
	equipmentPane.add(palantirImage);

	ringImage.resize(24, 24);
	equipmentPane.add(ringImage);

	virginImage.resize(24, 24);
	equipmentPane.add(virginImage);
/* END INVENTORY PANE LAYOUT */

    }

    public void UpdateStatusPane(int thePacket) {

	switch (thePacket) {

	case NAME_PACKET:
	    name = parent.readString();
	    if (name == null || name.length() == 0) {
		titleCanvas.changeHead("Welcome to Phantasia!");
		titleCanvas.changeTail("");
	    }
	    break;

	case LOCATION_PACKET:
	    titleCanvas.changeTail("( " + parent.readString() + " , " + parent.readString() + " )");
	    location = parent.readString();
	    if (name != null && name.length() > 0) {
		titleCanvas.changeHead(name + " is in " + location);
	    }
	    else {
		titleCanvas.changeHead("Character starts at:");
	    }
	    break;

	case ENERGY_PACKET:
	    energyGraph.changeStats(parent.readString(), parent.readLong(), parent.readLong());
	    break;

	case STRENGTH_PACKET:
	    strengthGraph.changeStats(parent.readString(), parent.readLong(), 0);
	    break;

	case SPEED_PACKET:
	    quicknessGraph.changeStats(parent.readString(), parent.readLong(), 0);
	    break;

	case SHIELD_PACKET:
	    shieldLabel.changeStats(parent.readString());
	    break;

	case SWORD_PACKET:
	    swordLabel.changeStats(parent.readString());
	    break;

	case QUICKSILVER_PACKET:
	    quicksilverLabel.changeStats(parent.readString());
	    break;

	case MANA_PACKET:
	    manaLabel.changeStats(parent.readString());
	    break;

	case LEVEL_PACKET:
	    levelLabel.changeStats(parent.readString());
	    break;

	case GOLD_PACKET:
	    goldLabel.changeStats(parent.readString());
	    break;

	case GEMS_PACKET:
	    gemsLabel.changeStats(parent.readString());
	    break;

	case CLOAK_PACKET:
	    cloakImage.setImage("cloak." + parent.readString() + ".gif");
	    break;

	case BLESSING_PACKET:
	    blessingImage.setImage("blessing." + parent.readString() + ".gif");
	    break;

	case CROWN_PACKET:
	    crownImage.setImage("crown." + parent.readString() + ".gif");
	    break;

	case PALANTIR_PACKET:
	    palantirImage.setImage("palantir." + parent.readString() + ".gif");
	    break;

	case RING_PACKET:
	    ringImage.setImage("ring." + parent.readString() + ".gif");
	    break;

	case VIRGIN_PACKET:
	    virginImage.setImage("virgin." + parent.readString() + ".gif");
	    break;

	default:
	    parent.errorDialog.bringUp("statusPane told to update a non-existant item.",
		    "item: " + thePacket, "The game will now terminate.");
	}
	repaint();
    }

    public void setImages() {

        shieldImage.setImage("shield.gif");
        swordImage.setImage("sword.gif");
        quicksilverImage.setImage("quicksilver.gif");
	manaImage.setImage("mana.gif");
	levelImage.setImage("level.gif");
	goldImage.setImage("gold.gif");
	gemsImage.setImage("gems.gif");
	cloakImage.setImage("cloak.No.gif");
	blessingImage.setImage("blessing.No.gif");
	crownImage.setImage("crown.No.gif");
	palantirImage.setImage("palantir.No.gif");
	ringImage.setImage("ring.No.gif");
	virginImage.setImage("virgin.No.gif");

    }
}
